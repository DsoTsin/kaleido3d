mod proto;
mod usbmuxd;
mod lockdownd;
mod ssl;

use std::thread::{Builder};
use std::{
    sync::{
        Arc,Mutex,
    }
};

use common::*;
use self::usbmuxd::{UsbmuxdSession,USBMUXD_HOST,UsbmuxdStream};
use self::lockdownd::LockDowndClient;

pub fn list_devices() -> Vec<Device> {
    let mut v = Vec::new();
    if let Ok(mut s) = UsbmuxdStream::connect(USBMUXD_HOST) {
        let mut session = UsbmuxdSession::new(&mut s);
        session.list_devices().map(|devs| {
            for dev in devs {
                session.read_pair_record(
                    dev.properties.udid.clone(), 
                    dev.device_id.clone() as _).map(|record| {
                        let mut s = UsbmuxdStream::connect(USBMUXD_HOST).unwrap();
                        let mut lockd = LockDowndClient::new(&mut s, 
                            dev.device_id as _, 
                            record.root_certificate.clone(), 
                            record.root_private_key.clone(), 
                            record.host_id.clone(), 
                            record.system_buid.clone(), 
                            "mix_device".into());
                        let dev_name = lockd.get_value("".into(), "DeviceName").unwrap();
                        let os_ver = lockd.get_value("".into(), "ProductVersion").unwrap();
                        let dev_type = lockd.get_value("".into(), "ProductType").unwrap();

                        v.push(Device::IOS(dev.properties.udid, 
                            IosDev {
                                id: dev.device_id as _,
                                root_cert: record.root_certificate,
                                root_pkey: record.root_private_key,
                                host_id: record.host_id,
                                sys_buid: record.system_buid,
                                name: dev_name,
                                os_ver: os_ver,
                                dtype: dev_type
                            }));
                        
                    }).unwrap();
            }
        }).unwrap();
    }
    v
}

pub fn syslog(udid: &String, dev_id: &u32, func: LogFn, usr_data: *mut libc::c_void) {
    if let Ok(mut s) = UsbmuxdStream::connect(USBMUXD_HOST) {
        let mut session = UsbmuxdSession::new(&mut s);
        let device_id = *dev_id;
        session.read_pair_record(udid.to_string(), device_id.clone()).map(|record| {
            let api = UserObject(
                UserObjectWrapper {
                    obj_ptr: usr_data,
                }
            );
            let shared_api = Arc::new(Mutex::new(api));
            let builder = Builder::new()
                .name("iOS_syslog".into())
                .stack_size(4 * 1024 * 1024);
            let handler = builder.spawn(move || {    
                let mut lockd = LockDowndClient::new(&mut s, 
                    device_id, 
                    record.root_certificate, 
                    record.root_private_key, 
                    record.host_id, 
                    record.system_buid, 
                    "idevicelog".into());
                lockd.start_service("com.apple.syslog_relay", |ns| {
                    let mut log_buf = [0u8; 4096];
                    loop {
                        use std::io::Read;
                        ns.read(&mut log_buf).map(|l| {
                            if l > 0 {
                                unsafe {
                                    if let Some(lfn) = func {
                                        let my_api = shared_api.lock().unwrap();
                                        lfn(log_buf.as_ptr() as _, l as _, my_api.0.obj_ptr);
                                    }
                                }
                            }
                        }).unwrap();
                    }
                });
            });
            // TODO handle
        }).unwrap();
    }
}

#[cfg(test)]
mod tests {
    use ios::usbmuxd::*;
    use ios::lockdownd::*;

    #[test]
    fn test_lockdownd() {
        super::list_devices();

        let mut s = UsbmuxdStream::connect(USBMUXD_HOST).unwrap();
        let mut session = UsbmuxdSession::new(&mut s);
        let mut _res = session.list_devices();
        let res = _res.unwrap();
        if res.len() > 0 {
            let dev = &res[0];
            // read pair record
            let record = UsbmuxdSession::new(&mut s).read_pair_record(dev.properties.udid.clone(), dev.device_id as _).unwrap();
            
            // create lockdownd
            let mut lockd = LockDowndClient::new(&mut s, 
                dev.device_id as _, 
                record.root_certificate.clone(), 
                record.root_private_key.clone(), 
                record.host_id.clone(), 
                record.system_buid.clone(), 
                "idevicelog".into());

            // lockdownd start log service
            use std::io::Read;
            lockd.start_service("com.apple.syslog_relay", |ns| {
                let mut log_buf = [0u8; 4096];
                let mut rsize = 0;
                loop {
                    ns.read(&mut log_buf).map(|le| {
                        log_buf[le] = 0;
                        let msg = String::from_utf8(log_buf.to_vec()).unwrap();
                        print!("{}", msg);
                        rsize += le;
                    }).unwrap();
                    if rsize > 20000 {
                        break;
                    }
                }
            });

            // GetValue
            let mut s = UsbmuxdStream::connect(USBMUXD_HOST).unwrap();
            let mut lockd = LockDowndClient::new(&mut s, 
                dev.device_id as _, 
                record.root_certificate.clone(), 
                record.root_private_key.clone(), 
                record.host_id.clone(), 
                record.system_buid.clone(), 
                "idevicelog".into());
            lockd.get_value("".into(), "DeviceName");
            lockd.get_value("".into(), "ProductVersion");
            lockd.get_value("".into(), "ProductType");
        }
    }
}