use super::usbmuxd::*;
use super::proto::*;
use common::error::{Error,ErrorKind,UsbMuxdError};
use plist::stream::{Event};
use std::result::Result;

pub struct LockDowndClient<'a> {
    connection  : Option<UsbmuxdSession<'a>>,
    device_id   : u32,
    root_cert   : String,
    root_pk     : String,        
    host_id     : String, 
    sys_buid    : String, 
    label       : String
}

//pub const LOCKDOWN_PROTOCOL_VERSION: &str = "2";

//"com.apple.mobile.mobile_image_mounter"
//"com.apple.syslog_relay"
//"com.apple.afc"
//"com.apple.instruments.server.services.graphics.opengl"
//"com.apple.instruments.remoteserver"
//"com.apple.instruments.remoteserver.DVTSecureSocketProxy" after ios14
//"Failed to start the instruments server (0x%x). "
//"Perhaps DeveloperDiskImage.dmg is not installed on the device?\n"
//@see https://github.com/troybowman/ios_instruments_client/blob/master/ios_instruments_client.cpp

pub enum LockdowndReturn {
    Session(bool,String), // enable ssl, session id
    Service(bool,i32), // enable service ssl, service port
}

/**
 * 1. lockdownd_new_with_handshake
 *      a. query type
 *      b. get_value "ProductVersion"
 *      c. read pair record
 *      d. optional pair
 *      e. start session
 */
impl<'a> LockDowndClient<'a> {
    pub fn new(s: &'a mut UsbmuxdStream, 
        device_id: u32, root_cert: String, root_pk: String,
        host_id: String, sys_buid: String, label: String
    ) -> Self {
        let mut session = UsbmuxdSession::new(s);
        match session.connect_device(device_id, 32498) {
            Ok(_) => LockDowndClient {
                connection  : Some(session),
                device_id   : device_id,
                root_cert   : root_cert,
                root_pk     : root_pk,
                host_id     : host_id,
                sys_buid    : sys_buid,
                label       : label
            },
            Err(_) => LockDowndClient {
                connection  : None,
                device_id   : device_id,
                root_cert   : root_cert,
                root_pk     : root_pk,
                host_id     : host_id,
                sys_buid    : sys_buid,
                label       : label
            }
        }
    }

    pub fn is_valid(&mut self) -> bool {
        self.connection.is_none()
    }

    pub fn query_type(&mut self) {
        // let plist = &[
        //     Event::StartDictionary(None),
        //     Event::String("Request".to_owned()),
        //     Event::String("QueryType".to_owned()),
        //     Event::String("Label".to_owned()),
        //     Event::String(self.label.clone()),
        //     Event::EndCollection,
        // ];

        // check Type
    }

    pub fn get_value(&mut self, domain: &str, key: &str) -> Option<String> {
        let plist = &[
            Event::StartDictionary(None),
            Event::String("Request".to_owned()),
            Event::String("GetValue".to_owned()),
            // Event::String("Domain".to_owned()),
            // Event::String(domain.to_owned()),
            Event::String("Key".to_owned()),
            Event::String(key.to_owned()),
            Event::String("Label".to_owned()),
            Event::String(self.label.clone()),
            Event::EndCollection,
        ];

        if let Some(session) = self.connection.as_mut() {
            session.plist_write(plist);
            match session.plist_read::<LockdowndGetValueRsp>() {
                Ok(rsp) => {
                    return Some(rsp.value);
                },
                Err(e) => {
                }
            }
        }
        None
    }

    pub fn pair(&mut self) {

    }

    /// host_id : @see PairRecord:host_id
    /// sys_buid: @see PairRecord:system_buid
    fn start_session<F>(&mut self, fn_start_service: F) -> Result<LockdowndReturn, Error>
        where F: Fn(&mut UsbmuxdSession, bool, &String) -> Result<LockdowndReturn, Error> {
        let plist = &[
            Event::StartDictionary(None),
            Event::String("Request".to_owned()),
            Event::String("StartSession".to_owned()),
            Event::String("HostID".to_owned()),     // optional
            Event::String(self.host_id.clone()),
            Event::String("SystemBUID".to_owned()), // optional
            Event::String(self.sys_buid.clone()),
            Event::String("Label".to_owned()),
            Event::String(self.label.clone()),
            Event::EndCollection,
        ];
        if let Some(session) = self.connection.as_mut() {
            session.plist_write(plist);
            match session.plist_read::<LockdowndStartSessionRsp>() {
                Ok(rsp) => {
                    if rsp.enable_session_ssl {
                        session.enable_ssl(&self.root_cert, &self.root_pk);
                    }
                    fn_start_service(session, rsp.enable_session_ssl, &rsp.session_id)
                },
                Err(e) => {
                    Err(e)
                }
            }
        } else {
            Err(ErrorKind::with_usbmuxd(UsbMuxdError::Unknown))
        }
    }

    pub fn start_service<F>(&mut self, service_name: &str, f: F) 
        where F: FnOnce(&mut UsbmuxdSession) -> () {
         self.start_session(|session, _, _| 
            -> Result<LockdowndReturn, Error> {
            let plist = &[
                Event::StartDictionary(None),
                Event::String("Request".to_owned()),
                Event::String("StartService".to_owned()),
                Event::String("Service".to_owned()),
                Event::String(service_name.to_owned()),
                Event::EndCollection,
            ];
            session.plist_write(plist);
            session.plist_read::<LockdowndStartServiceRsp>()
            .map_or_else(
                |e| Err(e),
                |rsp| Ok(LockdowndReturn::Service(rsp.enable_service_ssl,rsp.port))
            )
        }).map_or_else(
            |_| {},
            |ret| {
                if let LockdowndReturn::Service(ssl, port) = ret {
                    let mut s = UsbmuxdStream::connect(USBMUXD_HOST).unwrap();
                    let mut ns = UsbmuxdSession::new(&mut s);
                    let port_be = (port as u16).to_be();
                    ns.connect_device(self.device_id, port_be as u16)
                        .map_or_else(
                            |e| {
                                println!("Error to connect to service {}", e);
                            },
                            |_| {
                                if ssl {
                                    ns.enable_ssl(&self.root_cert, &self.root_pk);
                                }
                                f(&mut ns);
                            }
                        );
                }  
            }
        );
    }
}