use serde::{Serialize, Deserialize};
use std::fmt::{self, Formatter, Display};

#[derive(Debug, Copy, Clone, PartialEq, Eq)]
#[repr(u32)]
pub enum UsbMuxdResult {
    Ok = 0,
    BadCommand = 1,
    BadDev = 2,
    ConnRefused = 3,
    BadVersion = 6,
}

pub const RESULT_OK: UsbMuxdResult = UsbMuxdResult::Ok;
pub const RESULT_BADCOMMAND: UsbMuxdResult = UsbMuxdResult::BadCommand;
pub const RESULT_BADDEV: UsbMuxdResult = UsbMuxdResult::BadDev;
pub const RESULT_CONNREFUSED: UsbMuxdResult = UsbMuxdResult::ConnRefused;
pub const RESULT_BADVERSION: UsbMuxdResult = UsbMuxdResult::BadVersion;

#[derive(Debug, Copy, Clone, PartialEq, Eq)]
#[repr(u32)]
pub enum UsbMuxdMsgType {
    Result = 1,
    Connect = 2,
    Listen = 3,
    DeviceAdd = 4,
    DeviceRemove = 5,
    Plist = 8,
}

pub const MESSAGE_RESULT: UsbMuxdMsgType = UsbMuxdMsgType::Result;
pub const MESSAGE_CONNECT: UsbMuxdMsgType = UsbMuxdMsgType::Connect;
pub const MESSAGE_LISTEN: UsbMuxdMsgType = UsbMuxdMsgType::Listen;
pub const MESSAGE_DEVICE_ADD: UsbMuxdMsgType = UsbMuxdMsgType::DeviceAdd;
pub const MESSAGE_DEVICE_REMOVE: UsbMuxdMsgType = UsbMuxdMsgType::DeviceRemove;
pub const MESSAGE_PLIST: UsbMuxdMsgType = UsbMuxdMsgType::Plist;

#[repr(packed)]
#[derive(Serialize, Deserialize, Copy, Clone)]
pub struct UsbMuxdHeader {
    pub length: u32,
    pub version: u32,
    pub message: u32,
    pub tag: u32,
}

impl UsbMuxdHeader {
    pub fn new() -> Self {
        UsbMuxdHeader {
            length: 0,
            version: 0,
            message: 0,
            tag: 0,
        }
    }
}

#[repr(packed)]
#[derive(Serialize, Deserialize)]
pub struct UsbMuxdResultMsg {
    pub header: UsbMuxdHeader,
    pub result: u32,
}

#[repr(packed)]
#[derive(Serialize, Deserialize)]
pub struct UsbMuxdConnectRequest {
    pub header: UsbMuxdHeader,
    pub device_id: u32,
    pub port: u16,
    pub _reserved: u16,
}

#[repr(packed)]
#[derive(Serialize, Deserialize)]
pub struct UsbMuxdListenRequest {
    pub header: UsbMuxdHeader,
}

#[repr(packed)]
//#[derive(Serialize, Deserialize)]
pub struct UsbMuxdDeviceRecord {
    pub device_id: u32,
    pub product_id: u16,
    pub serial_number: [char; 256],
    pub _padding: u16,
    pub location: u32,
}

#[repr(packed)]
#[derive(Serialize, Deserialize)]
pub struct DTXMessageHeader
{
    pub magic : u32, //0x1F3D5B79 
    pub header_length : u32,
    pub fragment_id : u16,
    pub fragment_count: u16,
    pub payload_length : u32,
    pub message_id : u32,
    pub conversation_index : u32,
    pub channel_code : u32,
    pub expects_reply : u32
}

#[repr(packed)]
#[derive(Serialize, Deserialize)]
struct DTXMessagePayloadHeader
{
    pub flags : u32,
    pub auxiliary_length : u32,
    pub total_lenght: u64
}

#[derive(Deserialize, Clone)]
pub struct DeviceProps {
    // #[serde(rename = "ConnectionSpeed")]
    // pub connection_speed: i64,
    #[serde(rename = "ConnectionType")]
    pub connection_type: String,
    #[serde(rename = "DeviceID")]
    pub device_id: i64,
    #[serde(rename = "LocationID")]
    pub location_id: i64,
    #[serde(rename = "ProductID")]
    pub product_id: i64,
    #[serde(rename = "SerialNumber")]
    pub serial_number: String,
    #[serde(rename = "UDID")]
    pub udid: String,
    // #[serde(rename = "USBSerialNumber")]
    // pub usb_serial: String
}

#[derive(Deserialize,Clone)]
pub struct DeviceDesc {
    #[serde(rename = "DeviceID")]
    pub device_id: i64,
    #[serde(rename = "MessageType")]
    pub message_type: String,
    #[serde(rename = "Properties")]
    pub properties: DeviceProps
}

#[derive(Deserialize)]
#[serde(rename_all = "PascalCase")]
pub struct DeviceListResponse {
    pub device_list: Vec<DeviceDesc>
}

#[derive(Deserialize)]
#[serde(rename_all = "PascalCase")]
pub struct ReadPairRecordResponse {
    pub pair_record_data: String
}

#[derive(Deserialize)]
#[serde(rename_all = "PascalCase")]
pub struct ConnectResponse {
    pub message_type : String,
    pub number : i32
}

#[derive(Deserialize)]
pub struct PairRecord {
    #[serde(rename = "DeviceCertificate")]
    pub device_certificate: String,
    // #[serde(rename = "EscrowBag")] //lockdownd_start_service_with_escrow_bag, only used in backup
    // pub escrow_bag: String,
    #[serde(rename = "HostCertificate")]
    pub host_certificate: String,
    #[serde(rename = "HostID")]
    pub host_id: String,
    #[serde(rename = "HostPrivateKey")]
    pub host_private_key: String,
    #[serde(rename = "RootCertificate")]
    pub root_certificate: String,
    #[serde(rename = "RootPrivateKey")]
    pub root_private_key: String,
    #[serde(rename = "SystemBUID")]
    pub system_buid: String,
    #[serde(rename = "WiFiMACAddress")]
    pub wifi_mac_address: String,
}

impl Display for PairRecord {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        write!(f, "DeviceCert : {}\nHostCert : {}\nWiFiMAC : {}\n",
               self.device_certificate, self.host_certificate, self.wifi_mac_address)
    }
}

#[derive(Deserialize)]
pub struct LockdowndStartSessionRsp {
    #[serde(rename = "EnableSessionSSL")]
    pub enable_session_ssl : bool,
    #[serde(rename = "Request")]
    pub request : String,
    #[serde(rename = "SessionID")]
    pub session_id : String,
}

#[derive(Deserialize)]
pub struct LockdowndStartServiceRsp {
    #[serde(rename = "EnableServiceSSL")]
    pub enable_service_ssl : bool,
    #[serde(rename = "Port")]
    pub port : i32,
    #[serde(rename = "Request")]
    pub request : String,
    #[serde(rename = "Service")]
    pub service : String
}

#[derive(Deserialize)]
pub struct LockdowndGetValueRsp {
    #[serde(rename = "Value")]
    pub value : String,
    #[serde(rename = "Request")]
    pub request : String
}

pub const USBMUXD_PROTOCOL_VERSION: i32 = 1;
pub const PLIST_BUNDLE_ID : &'static str = "org.tsinworks.usbmuxd";
pub const PLIST_CLIENT_VERSION_STRING : &'static str = "usbmuxd";
pub const PLIST_PROGNAME : &'static str = "libusbmuxd";
pub const PLIST_LIBUSBMUX_VERSION : u64 = 3;
pub const LOCKDOWND_PORT : u16 = 0xf27e;