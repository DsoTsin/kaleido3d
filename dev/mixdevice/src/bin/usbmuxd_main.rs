// mod lib;
// use crate::lib::*;
//extern crate mix_device;
// use mix_device::ios::usbmuxd::{UsbmuxdSession,USBMUXD_HOST,UsbmuxdStream};
// use mix_device::ios::lockdownd::*;
fn main() {
    // let mut s = UsbmuxdStream::connect(USBMUXD_HOST).unwrap();
    // let mut session = UsbmuxdSession::new(&mut s);
    // let mut _res = session.list_devices();
    // let res = _res.unwrap();
    // if res.len() > 0 {
    //     let dev = &res[0];
    //     // read pair record
    //     let record = UsbmuxdSession::new(&mut s).read_pair_record(dev.properties.udid.clone(), dev.device_id as _).unwrap();
        
    //     // create lockdownd
    //     let mut lockd = LockDowndClient::new(&mut s, 
    //         dev.device_id as _, 
    //         record.root_certificate, 
    //         record.root_private_key, 
    //         record.host_id, 
    //         record.system_buid, 
    //         "idevicelog".into());

    //     lockd.start_session();
    // }
}