extern crate bincode;
use std::{fmt,ops,cmp,borrow};
use std::collections::HashMap;
pub mod error;
//use std::sync::Arc;

#[repr(i32)]
#[derive(Debug, Copy, Clone)]
pub enum MixError
{
    MixSucceed = 0,
    MixFailedToList = -1,
    MixUnknownError = -1001,
    MixFailedToConnect = -1002,
    MixFailedToForward = -1003,
    MixFailedToExecute = -1004,
    MixFailedToListAndroid = -1005,
    MixFailedToListIOS = -1006,
    MixFailedInvalidArg = -1007,
}

#[repr(u32)]
#[derive(Debug, Copy, Clone)]
pub enum MixDevicePlatform
{
    DevicePlatformIOS = 0,
    DevicePlatformAndroid = 1,
    Unsupported = 3
}

#[repr(C)]
pub struct Handle<T>(*mut T);

impl<T> Handle<T> {
    pub fn new(val: T)->Self {
        let ptr = Box::into_raw(Box::new(val));
        Handle(ptr)
    }
    pub fn null() -> Self {
        Handle(0 as *mut _)
    }
    pub fn unbox(self) -> Option<T> {
        if self.0 == 0 as *mut T {
            None
        } else {
            Some(*unsafe {Box::from_raw(self.0)})
        }
    } 
    pub fn as_ref(&self) -> Option<&T> {
        if self.0 == 0 as *mut T {
            None
        } else {
            Some(unsafe { &*self.0 })
        }
    }
}

impl<T> Clone for Handle<T> {
    fn clone(&self) -> Self {
        Handle(self.0)
    }
}

impl<T> Copy for Handle<T> {}

impl<T> ops::Deref for Handle<T> {
    type Target = T;
    fn deref(&self) -> &T {
        unsafe { &*self.0 }
    }
}

impl<T> ops::DerefMut for Handle<T> {
    fn deref_mut(&mut self) -> &mut T {
        unsafe { &mut *self.0 }
    }
}

impl<T> borrow::Borrow<T> for Handle<T> {
    fn borrow(&self) -> &T {
        unsafe { &*self.0 }
    }
}

impl<T> cmp::PartialEq for Handle<T> {
    fn eq(&self, other: &Self) -> bool {
        self.0.eq(&other.0)
    }
}

impl<T> fmt::Debug for Handle<T> {
    fn fmt(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
        write!(formatter, "Handle({:p})", self.0)
    }
}

pub struct UserObjectWrapper {
    pub obj_ptr: *mut libc::c_void,
}

pub struct UserObject(pub UserObjectWrapper);
unsafe impl Send for UserObject {}

pub struct AndroDev {
    pub name: String,
    pub version: String,
    pub andro_ver: String
}

pub struct IosDev {
    pub id : u32,

    pub root_cert: String,
    pub root_pkey: String,
    pub host_id: String,
    pub sys_buid: String,
    
    /// Device name
    pub name: String,
    pub os_ver: String,
    /// Device type, eg. iPad8,1
    pub dtype: String,
}

pub enum Device {
    /// udid
    IOS(String, IosDev),
    /// serial id
    Android(String, AndroDev),
    HarmonyOS,
}

pub type LogFn = Option<unsafe extern "C" fn(*const libc::c_char, libc::c_int, *mut libc::c_void) -> libc::c_void>;
pub type ProgressFn = Option<unsafe extern "C" fn(u32, *mut libc::c_void) -> libc::c_void>;