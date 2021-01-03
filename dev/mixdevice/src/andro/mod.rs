use std::mem;
use std::io::{Write,Read,Result,Cursor};
use serde::{Serialize, Deserialize};

mod adb;

use common::{Device, LogFn};

pub fn list_devices() -> Vec<Device>
{
    adb::list_devices()
}

pub fn log_start(dev_serial: &String, func: LogFn, usr_data: *mut libc::c_void) {
    adb::logcat(dev_serial, func, usr_data);
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_adb() {
        adb::list_devices();
    }
}