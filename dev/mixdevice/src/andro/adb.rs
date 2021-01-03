use std::net::TcpStream;
use std::io::{Write,Read};
use common::*;
use std::thread::{Builder};
use std::{
    sync::{
        Arc,Mutex,
    }
};
use regex::Regex;
use std::collections::HashMap;

const ADB_HOST: &'static str = "127.0.0.1:5037";

fn format_request(command: &str) -> String {
    format!("{:04x}{}", command.len(), command)
}

fn hex_u(c: u8) -> u32 {
    let ic = c as u32;
    if ic >= 0x30 && ic <= 0x39 {
        ic - 0x30
    } else if ic >= 0x61 && ic <= 0x66 { // lower case
        ic + 9 - 0x61
    } else if ic >= 0x41 && ic <= 0x46 {
        ic + 9 - 0x41
    } else {
        0
    }
}

fn hex_to_u32(bytes:&[u8;4]) -> u32 {
    4096 * hex_u(bytes[0])
        + 256 * hex_u(bytes[1]) 
        + 16 *  hex_u(bytes[2]) 
        + hex_u(bytes[3])
}

fn is_okay(bytes:&[u8;4]) -> bool {
    bytes[0]==('O' as _)
        && bytes[1]==('K' as _)
        && bytes[2]==('A' as _)
        && bytes[3]==('Y' as _)
}

pub fn list_devices() -> Vec<Device> {
    let mut devs = Vec::new();
    if let Ok(mut s) = TcpStream::connect(ADB_HOST) {
        s.write_all(format_request("host:track-devices").as_bytes()).unwrap();
        let mut rsp = [0; 4];
        s.read(&mut rsp).unwrap();
        if is_okay(&rsp)  {
            // read payload length in hex string
            s.read(&mut rsp).unwrap();
            let len = hex_to_u32(&rsp);
            let mut chars = Vec::new();
            chars.resize(len as _, 0 as u8);
            s.read_exact(&mut chars).map(|_| {
                for dev_line in chars.split(|c| *c==0x0A) {
                    let v = dev_line.to_vec();
                    let mut slices = v.split(|c| *c==0x09);
                    let serial = slices.next().unwrap();
                    if serial.len() > 0 {
                        let serial = String::from_utf8(serial.to_vec()).unwrap();
                        let model = read_device_prop(&serial, "ro.product.model");
                        let sdk_ver = read_device_prop(&serial, "ro.build.version.sdk");
                        let release = read_device_prop(&serial, "ro.build.version.release");
                        devs.push(Device::Android(serial, 
                            AndroDev {
                                name: model.unwrap(),
                                version: sdk_ver.unwrap(),
                                andro_ver: release.unwrap()
                            }));
                    }
                }
            }).unwrap();
        }
    }
    devs
}

pub fn read_device_prop(dev_serial: &String, key: &str) -> Option<String> {
    if let Ok(mut s) = TcpStream::connect(ADB_HOST) {
        s.write_all(format_request(format!("host:transport:{}", dev_serial).as_str()).as_bytes()).unwrap();
        let mut rsp = [0; 4];
        s.read(&mut rsp).unwrap();
        if is_okay(&rsp)  {
            let req = format!("shell:getprop {}", key);
            s.write_all(format_request(req.as_str()).as_bytes()).unwrap();
            s.read(&mut rsp).unwrap();
            if is_okay(&rsp) {
                let mut ful_buf = Vec::new();
                loop {
                    let mut buf = [0u8;4096];
                    match s.read(&mut buf) {
                        Ok(l) => { 
                            if l > 0 {
                                ful_buf.extend_from_slice(&buf[0..l]);
                            } else {
                                break;
                            }
                        },
                        Err(_) => break
                    }
                }
                let mut rstr = String::from_utf8(ful_buf).unwrap();
                let len = rstr.len();
                rstr.truncate(len - 1);
                return Some(rstr);
            }
        }
    }
    None
}

pub fn logcat(dev_serial: &String, 
    func: LogFn, usr_data: *mut libc::c_void) {
    if let Ok(mut s) = TcpStream::connect(ADB_HOST) {
        s.write_all(format_request(format!("host:transport:{}", dev_serial).as_str()).as_bytes()).unwrap();
        let mut rsp = [0; 4];
        s.read(&mut rsp).unwrap();
        if is_okay(&rsp)  {
            s.write_all(format_request("shell:logcat").as_bytes()).unwrap();
            s.read(&mut rsp).unwrap();
            if is_okay(&rsp) {
                let api = UserObject(
                    UserObjectWrapper {
                        obj_ptr: usr_data,
                    }
                );
                let shared_api = Arc::new(Mutex::new(api));
                // read all logs
                let builder = Builder::new()
                    .name("AdbLogCat".into())
                    .stack_size(2048 * 1024);
                let handler = builder.spawn(move || {
                    // thread code
                    loop {
                        let mut log_buf = [0u8; 4096];
                        s.read(&mut log_buf).map(|l| {
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
                }).unwrap();
                //TODO handler.join().unwrap();
            }
        }
    }
}


pub fn read_device_props(dev_serial: &String) -> Option<HashMap<String,String>> {
    if let Ok(mut s) = TcpStream::connect(ADB_HOST) {
        s.write_all(format_request(format!("host:transport:{}", dev_serial).as_str()).as_bytes()).unwrap();
        let mut rsp = [0; 4];
        s.read(&mut rsp).unwrap();
        if is_okay(&rsp)  {
            s.write_all(format_request("shell:getprop ro.product.name").as_bytes()).unwrap();
            s.read(&mut rsp).unwrap();
            if is_okay(&rsp) {
                let mut ful_buf = Vec::new();
                loop {
                    let mut buf = [0u8;16384];
                    match s.read(&mut buf) {
                        Ok(l) => { 
                            if l > 0 {
                                ful_buf.extend_from_slice(&buf[0..l]);
                            } else {
                                break;
                            }
                        },
                        Err(_) => break
                    }
                }
                return parse_prop(&mut ful_buf);
            }
        }
    }
    None
}

lazy_static! {
    static ref REGLN: Regex = Regex::new(r"\[([\d\w\.]*)\]\s*:\s*\[([\d\w\.]*)\]").unwrap();
}

fn parse_prop(buf: &mut [u8]) -> Option<HashMap<String,String>>
{
    let mut map = HashMap::new();
    //std::str::from_utf8(buf)
    for cap in REGLN.captures_iter(std::str::from_utf8(buf).unwrap()) {
        map.insert(cap[1].into(), cap[2].into());
    }
    if map.len() > 0 {
        Some(map)
    } else {
        None
    }
}