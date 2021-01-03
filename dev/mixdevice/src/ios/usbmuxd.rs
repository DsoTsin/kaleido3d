use std::mem;
use std::sync::atomic::{AtomicU32, Ordering};
use std::sync::Arc;
use std::io::{Write,Read,Seek,Cursor,BufReader};
use std::slice::Iter;
use std::result::Result;
use common::error::{Error,ErrorKind};

use super::ssl::*;
use super::proto::*;
use plist::stream::{Event,Writer,XmlWriter};
use serde::{Serialize, Deserialize};

#[cfg(not(target_os = "windows"))]
pub const USBMUXD_HOST: &'static str = "/var/run/usbmuxd";
#[cfg(not(target_os = "windows"))]
pub type UsbmuxdStream = std::os::unix::net::UnixStream;

#[cfg(target_os = "windows")]
pub const USBMUXD_HOST : &'static str = "127.0.0.1:27015";
#[cfg(target_os = "windows")]
pub type UsbmuxdStream = std::net::TcpStream;

pub struct UsbmuxdSession<'a> {
    stream: &'a mut UsbmuxdStream,
    seq: AtomicU32,
    /// can be 1 (plist) or 2 (binary)
    proto_ver : i32,
    ssl_ctx: Option<Context>,
    ssl: Option<Ssl>
}

fn create_xml_plist_msg(msg: &str) -> String {
    let plist = &[
        Event::StartDictionary(None),
        Event::String("BundleID".to_owned()),
        Event::String(PLIST_BUNDLE_ID.to_owned()),
        Event::String("ClientVersionString".to_owned()),
        Event::String(PLIST_CLIENT_VERSION_STRING.to_owned()),
        Event::String("MessageType".to_owned()),
        Event::String(msg.to_owned()),
        Event::String("ProgName".to_owned()),
        Event::String(PLIST_PROGNAME.to_owned()),
        Event::String("kLibUSBMuxVersion".to_owned()),
        Event::Integer(PLIST_LIBUSBMUX_VERSION.into()),
        Event::EndCollection,
    ];
    let mut cursor = Cursor::new(Vec::new());
    {
        let mut plist_w = XmlWriter::new(&mut cursor);
        for item in plist {
            plist_w.write(item).unwrap();
        }
    }
    String::from_utf8(cursor.into_inner()).unwrap()
}

//ReadPairRecord
//SavePairRecord
impl<'a> UsbmuxdSession<'a> {
    pub fn new(stream : &'a mut UsbmuxdStream) -> Self {
        UsbmuxdSession {
            stream      : stream,
            seq         : AtomicU32::new(0),
            proto_ver   : 1,
            ssl_ctx     : None,
            ssl         : None
        }
    }

    pub fn link_ssl(&mut self, ssl: &mut Ssl) {
        ssl.set_stream(&mut self.stream);
    }

    pub fn enable_ssl(&mut self, root_cer: &String, root_pk: &String) {
        let mut context = Context::new();
        context.add_certificate(root_cer);
        context.add_rsa_pkey(root_pk);
        let mut ssl = Ssl::new(&mut context);
        ssl.set_stream(&mut self.stream);
        self.ssl_ctx = Some(context);
        self.ssl = Some(ssl);
    }

    pub fn list_devices(&mut self) -> Result<Vec<DeviceDesc>, Error> {
        self.send_plist_packet(create_xml_plist_msg("ListDevices").as_bytes());
        self.seq.fetch_add(1, Ordering::AcqRel);
        self.on_list_device_packet()
    }

    // for lockdownd creation
    pub fn read_pair_record(&mut self, udid: String, device_id : u32) -> Result<PairRecord, Error> {
        let plist = &[
            Event::StartDictionary(None),
            Event::String("BundleID".to_owned()),
            Event::String(PLIST_BUNDLE_ID.to_owned()),
            Event::String("ClientVersionString".to_owned()),
            Event::String(PLIST_CLIENT_VERSION_STRING.to_owned()),
            Event::String("MessageType".to_owned()),
            Event::String("ReadPairRecord".to_owned()),
            Event::String("ProgName".to_owned()),
            Event::String(PLIST_PROGNAME.to_owned()),
            Event::String("kLibUSBMuxVersion".to_owned()),
            Event::Integer(PLIST_LIBUSBMUX_VERSION.into()),
            Event::String("PairRecordID".to_owned()),
            Event::String(udid),
            //"PairRecordData"
            //"DeviceID"
            Event::EndCollection,
        ];
        if device_id > 0 {
            // TODO
        }
        let mut cursor = Cursor::new(Vec::new());
        {
            let mut plist_w = XmlWriter::new(&mut cursor);
            for item in plist {
                plist_w.write(item).unwrap();
            }
        }
        self.send_plist_packet(String::from_utf8(cursor.into_inner()).unwrap().as_bytes());
        self.seq.fetch_add(1, Ordering::AcqRel);
        self.on_read_pair_record()
    }

    pub fn connect_device(&mut self, device_id: u32, port: u16) -> Result<i32, Error> {
        if self.proto_ver == 1 {
            let plist = &[
                Event::StartDictionary(None),
                Event::String("BundleID".to_owned()),
                Event::String(PLIST_BUNDLE_ID.to_owned()),
                Event::String("ClientVersionString".to_owned()),
                Event::String(PLIST_CLIENT_VERSION_STRING.to_owned()),
                Event::String("MessageType".to_owned()),
                Event::String("Connect".to_owned()),
                Event::String("DeviceID".to_owned()),
                Event::Integer(device_id.into()),
                Event::String("PortNumber".to_owned()),
                Event::Integer(port.into()),                
                Event::String("ProgName".to_owned()),
                Event::String(PLIST_PROGNAME.to_owned()),
                Event::String("kLibUSBMuxVersion".to_owned()),
                Event::Integer(PLIST_LIBUSBMUX_VERSION.into()),
                Event::EndCollection,
            ];
            let mut cursor = Cursor::new(Vec::new());
            {
                let mut plist_w = XmlWriter::new(&mut cursor);
                for item in plist {
                    plist_w.write(item).unwrap();
                }
            }
            self.send_plist_packet(String::from_utf8(cursor.into_inner()).unwrap().as_bytes());
        } else {
            let req = UsbMuxdConnectRequest {
                header : UsbMuxdHeader {
                    length: mem::size_of::<UsbMuxdConnectRequest>() as u32,
                    version : 2,
                    message: MESSAGE_CONNECT as _,
                    tag : self.seq.load(Ordering::Relaxed)
                },
                device_id,
                port,
                _reserved: 0
            };
            let bytes = bincode::serialize(&req);
            self.stream.write(bytes.unwrap().as_slice());
        }
        self.seq.fetch_add(1, Ordering::AcqRel);
        match self.get_response_header() {
            Ok(h) => {
                let sz_payload = h.length as i64 - mem::size_of::<UsbMuxdHeader>() as i64;
                if sz_payload >= 0 {
                    if h.message == MESSAGE_RESULT as u32 {
                        Ok(0)
                    } else if h.message == MESSAGE_PLIST as u32 {
                        let mut buff = Vec::new();
                        buff.resize(sz_payload as _, 0);
                        self.stream.read_exact(&mut buff[..]);
                        match plist::from_bytes::<ConnectResponse>(&buff) {
                                Ok(res) => {
                                    match res.number {
                                        0 => Ok(0),
                                        _ => Err(ErrorKind::with_usbmuxd(res.number.into()))
                                    }
                                },
                                Err(e) => {
                                    Err(ErrorKind::with_plist(e))
                                }
                        }
                    } else {
                        Ok(0)
                    }
                } else {
                    Ok(0)
                }
            },
            Err(err) => {
                Err(ErrorKind::with_bin(err))
            }
        }
    }

    fn send_raw_packet(&mut self, payload : &[u8]) {
        self.stream.write_all(payload);
    }

    pub fn plist_write(&mut self, plist: &[plist::stream::Event]) {    
        let mut cursor = Cursor::new(Vec::new());
        {
            let mut plist_w = XmlWriter::new(&mut cursor);
            for item in plist {
                plist_w.write(item).unwrap();
            }
        }
        let vec = cursor.into_inner();
        let len = vec.len() as u32;
        self.write(&len.to_be_bytes());
        self.write(&vec);
    }

    pub fn plist_read<T: serde::de::DeserializeOwned>(&mut self) -> Result<T, Error> {
        let mut buf = [0u8; 4];
        self.read_exact(&mut buf)
            .map_or_else(
                |ef| Err(ErrorKind::with_err(ErrorKind::Io(ef))),
                |_| {
                    let size = u32::from_be_bytes(buf);
                    let mut buff = Vec::new();
                    buff.resize(size as _, 0);
                    self.read_exact(&mut buff)
                        .map_or_else(
                            |ie| Err(ErrorKind::with_err(ErrorKind::Io(ie))),
                            |_| {
                                match plist::from_bytes::<T>(&buff) {
                                    Ok(r) => Ok(r),
                                    Err(pe) => Err(ErrorKind::with_plist(pe))
                                }
                            }
                        )
                }
            )
    }

    fn send_plist_packet(&mut self, payload : &[u8]) {
        let mut header =
            UsbMuxdHeader {
                length: mem::size_of::<UsbMuxdHeader>() as u32,
                version: USBMUXD_PROTOCOL_VERSION as u32,
                message: MESSAGE_PLIST as u32,
                tag : self.seq.load(Ordering::Relaxed)
            };
        if payload.len() > 0 {
            header.length += payload.len() as u32;
        }
        let bytes = bincode::serialize(&header).unwrap();
        self.stream.write(bytes.as_slice());
        self.stream.write_all(payload);
    }

    fn on_list_device_packet(&mut self) -> Result<Vec<DeviceDesc>, Error> {
        match self.get_response_header() {
            Ok(h) => {
                let sz_payload = h.length as i64 - mem::size_of::<UsbMuxdHeader>() as i64;
                if sz_payload >= 0 && h.message == MESSAGE_PLIST as u32 {
                    let mut buff = Vec::new();
                    buff.resize(sz_payload as _, 0);
                    self.stream.read_exact(&mut buff[..]);
                    match plist::from_bytes::<DeviceListResponse>(&buff) {
                        Ok(rsp) => { 
                            Ok(rsp.device_list) 
                        }
                        Err(err) => { 
                            Err(ErrorKind::with_plist(err))
                        }
                    }
                } else {
                    Err(ErrorKind::with_err(ErrorKind::UsbmuxdResponsePayloadEmpty))
                }
            }
            Err(err) => {
                Err(ErrorKind::with_bin(err))
            }
        }
    }

    fn on_read_pair_record(&mut self) -> Result<PairRecord, Error>  {
        match self.get_response_header() {
            Ok(h) => {
                let sz_payload = h.length as i64 - mem::size_of::<UsbMuxdHeader>() as i64;
                if sz_payload >= 0 && h.message == MESSAGE_PLIST as u32 {
                    let mut buff = Vec::new();
                    buff.resize(sz_payload as _, 0);
                    self.stream.read_exact(&mut buff[..]);
                    match plist::from_bytes::<ReadPairRecordResponse>(&buff) {
                        Ok(rsp) => { 
                            match plist::from_bytes::<PairRecord>(rsp.pair_record_data.as_bytes()) {
                                Ok(rec) => Ok(rec),
                                Err(err) => Err(ErrorKind::with_plist(err))
                            }
                        },
                        Err(err) => {
                            Err(ErrorKind::with_plist(err))
                        }
                    }
                } else {
                    Err(ErrorKind::with_err(ErrorKind::UsbmuxdResponsePayloadEmpty))
                }
            }
            Err(err) => {
                Err(ErrorKind::with_bin(err))
            }
        }
    }

    fn get_response_header(&mut self) -> bincode::Result<UsbMuxdHeader> {
        let rsp : bincode::Result<UsbMuxdHeader> = bincode::deserialize_from(
            self.stream.take(mem::size_of::<UsbMuxdHeader>() as u64));
        rsp
    }
}

impl<'a> Write for UsbmuxdSession<'a> {
    fn write(&mut self, buf: &[u8]) -> std::io::Result<usize> {
        match self.ssl.as_mut() {
            Some(ssl) => ssl.write(buf),
            None => self.stream.write(buf)
        }
    }

    fn flush(&mut self) -> std::io::Result<()> {
        Ok(())
    }
}

impl<'a> Read for UsbmuxdSession<'a> {
    fn read(&mut self, buf: &mut [u8]) -> std::io::Result<usize> {
        match self.ssl.as_mut() {
            Some(ssl) => ssl.read(buf),
            None => self.stream.read(buf)
        }
    }
}