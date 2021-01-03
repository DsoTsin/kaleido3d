use libc::*;
//use std::ptr;
use std::{thread, time};

#[cfg(unix)]
use std::os::unix::io::AsRawFd;
#[cfg(windows)]
use std::os::windows::io::AsRawSocket;

use super::usbmuxd::UsbmuxdStream;

#[repr(transparent)]
struct BIO(i32);

#[repr(transparent)]
struct BIOMethod(i32);

#[repr(transparent)]
struct SSLMethod(i32);

#[repr(transparent)]
struct SSLContext(i32);

#[repr(transparent)]
struct SSL(i32);

#[repr(transparent)]
struct X509(i32);

#[repr(transparent)]
struct RSA(i32);

const SSL_ERROR_NONE                : c_int = 0;
const SSL_ERROR_SSL                 : c_int = 1;
const SSL_ERROR_WANT_READ           : c_int = 2;
const SSL_ERROR_WANT_WRITE          : c_int = 3;
const SSL_ERROR_WANT_X509_LOOKUP    : c_int = 4;
const SSL_ERROR_SYSCALL             : c_int = 5;
const SSL_ERROR_ZERO_RETURN         : c_int = 6;
const SSL_ERROR_WANT_CONNECT        : c_int = 7;
const SSL_ERROR_WANT_ACCEPT         : c_int = 8;
const SSL_ERROR_WANT_CLIENT_HELLO_CB: c_int = 11;

#[link(name="libssl")]
#[link(name="libcrypto")]
extern "C" {
    fn BIO_s_socket() -> *const BIOMethod;
    fn BIO_new(m : * const BIOMethod) -> *mut BIO;
    fn BIO_free(a : * mut BIO) -> i32;
    fn BIO_int_ctrl(bp : *mut BIO, cmd : i32, larg : u64, iarg : i32) -> i32;
    fn TLS_method() -> * const SSLMethod;
    fn SSL_CTX_new(meth: * const SSLMethod) -> *mut SSLContext;
    fn SSL_CTX_free(ctx : * mut SSLContext);
    fn BIO_new_mem_buf(buf: *const u8, len : i32) -> *mut BIO;
    fn PEM_read_bio_X509(buf: *mut BIO, x: *mut *mut X509, passwd: *const u8, user: *mut libc::c_void) -> *mut X509;
    fn SSL_CTX_use_certificate(ctx: *mut SSLContext, x: *mut X509) -> i32;
    fn X509_free(x : *mut X509);
    fn PEM_read_bio_RSAPrivateKey(buf: *mut BIO, r: *mut *mut RSA, passwd: *mut libc::c_void, user: *mut libc::c_void)-> *mut RSA;
    fn SSL_CTX_use_RSAPrivateKey(ctx: *mut SSLContext, rsa: *mut RSA) -> i32;
    fn RSA_free(r: *mut RSA);

    fn SSL_new(ctx: *mut SSLContext) -> *mut SSL;
    fn SSL_free(s: *mut SSL);
    
    fn SSL_set_connect_state(s: *mut SSL);
    fn SSL_set_verify(s: *mut SSL, mode: i32, verify_callback: Option<extern "C" fn(i32, *mut X509) -> i32>);
    fn SSL_set_bio(s: *mut SSL, rbio: *mut BIO, wbio: *mut BIO);

    fn SSL_do_handshake(s: *mut SSL) -> i32;
    fn SSL_get_error(s: *const SSL, ret_code: i32) -> i32;
    fn SSL_get_version(s: *const SSL) -> *const c_char;
    fn SSL_shutdown(s: *mut SSL) -> i32;

    fn SSL_write(s: *mut SSL, buf: *const u8, num: i32) -> i32;
    fn SSL_pending(s: *const SSL) -> i32;
    fn SSL_read(s: *mut SSL, buf: *mut u8, num: i32) -> i32;
}

pub struct Ssl(*mut SSL);
pub struct Context(*mut SSLContext);

unsafe impl Send for Ssl {}
unsafe impl Send for Context {}

impl Context {
    pub fn new() -> Self {
        unsafe {
            Context(SSL_CTX_new(TLS_method()))
        }
    }

    pub fn add_rsa_pkey(&self, key: &String) {
        unsafe {
            let mut root_priv_key : *mut RSA = std::mem::uninitialized();
            //let bytes = key.as_bytes_mut();
            let membp = Box::new(BIO_new_mem_buf(key.as_ptr(), key.len() as _));
            PEM_read_bio_RSAPrivateKey(*membp.as_ref(), &mut root_priv_key, 0 as _, 0 as _);
            BIO_free(*membp.as_ref());
            let new_rpk = Box::new(root_priv_key);
            if SSL_CTX_use_RSAPrivateKey(self.0, *new_rpk.as_ref()) != 1 {
                println!("WARNING: Could not load RootPrivateKey");
            }
            RSA_free(*new_rpk);
        }
    }

    pub fn add_certificate(&self, cert: &String) {
        unsafe {
            let mut root_cert : *mut X509 = std::mem::uninitialized();
            let membp = Box::new(BIO_new_mem_buf(cert.as_bytes().as_ptr(), cert.len() as _));
            PEM_read_bio_X509(*membp.as_ref(), &mut root_cert, 0 as _, 0 as _);
            BIO_free(*membp.as_ref());
            //let new_rcer = Box::new(root_cert);
            if SSL_CTX_use_certificate(self.0, root_cert) != 1 {
                 println!("WARNING: Could not load RootCertificate");
            }
            X509_free(root_cert);
        }
    }
}

impl Drop for Context {
    fn drop(&mut self) {
        unsafe{
            SSL_CTX_free(self.0);
        }
    }
}

impl Ssl {
    pub fn new(ctx: &mut Context) -> Self {
        unsafe {
            Ssl(SSL_new(ctx.0))
        }
    }

    pub fn set_stream(&self, s: &mut UsbmuxdStream) -> i32 {
        unsafe {        
            let ssl_bio = BIO_new(BIO_s_socket());
	        BIO_int_ctrl(ssl_bio, 104, 0/* BIO_NOCLOSE */, s.as_raw_socket() as _);
            SSL_set_connect_state(self.0);
            SSL_set_verify(self.0, 0, None);
            SSL_set_bio(self.0, ssl_bio, ssl_bio);
            let mut ssl_error = 0;
            loop {
                ssl_error = SSL_get_error(self.0, SSL_do_handshake(self.0));
                if ssl_error == 0 || ssl_error != SSL_ERROR_WANT_READ {
                    break;
                }
                let ten_millis = time::Duration::from_millis(10);
                thread::sleep(ten_millis);
                // #ifdef WIN32
                // Sleep(100);
                // #else
                // struct timespec ts = { 0, 100000000 };
                // nanosleep(&ts, NULL);
                // #endif
            }
            ssl_error
            // if ssl_error != 0 {
            //     //debug_info("ERROR during SSL handshake: %s", ssl_error_to_string(ssl_error));
            //     SSL_free(ssl);
            //     SSL_CTX_free(ssl_ctx);
            // } else {
            //     ssl_data_t ssl_data_loc = (ssl_data_t)malloc(sizeof(struct ssl_data_private));
            //     ssl_data_loc->session = ssl;
            //     ssl_data_loc->ctx = ssl_ctx;
            //     connection->ssl_data = ssl_data_loc;
            //     ret = IDEVICE_E_SUCCESS;
            //     //debug_info("SSL mode enabled, %s, cipher: %s", SSL_get_version(ssl), SSL_get_cipher(ssl));
            // }
        }
    }

    pub fn shutdown(&self) {
        unsafe{
            SSL_shutdown(self.0);
        }
    }

    fn get_error(&mut self, ret: c_int) -> c_int {
        unsafe {
            SSL_get_error(self.0, ret)
        }
    }

    // fn make_error(&mut self, ret: c_int) -> Error {
    //     let code = self.get_error(ret);
    //     let cause = match code {
    //         ErrorCode::SSL => Some(InnerError::Ssl(ErrorStack::get())),
    //         ErrorCode::SYSCALL => {
    //             let errs = ErrorStack::get();
    //             if errs.errors().is_empty() {
    //                 self.get_bio_error().map(InnerError::Io)
    //             } else {
    //                 Some(InnerError::Ssl(errs))
    //             }
    //         }
    //         ErrorCode::ZERO_RETURN => None,
    //         ErrorCode::WANT_READ | ErrorCode::WANT_WRITE => {
    //             self.get_bio_error().map(InnerError::Io)
    //         }
    //         _ => None,
    //     };

    //     Error { code, cause }
    // }
}


impl std::io::Read for Ssl {
    fn read(&mut self, buf: &mut [u8]) -> std::io::Result<usize> {
        loop {
            unsafe {
                let ret = SSL_read(self.0, buf.as_mut_ptr(), buf.len() as _);
                if ret <= 0 {
                    let sslerr = SSL_get_error(self.0, ret);
                    match sslerr {
                        SSL_ERROR_WANT_READ => {},
                        SSL_ERROR_ZERO_RETURN => return Ok(0),
                        _ => {}
                    }
                } else {
                    return Ok(ret as _);
                }
            }
        }
    }
}

impl std::io::Write for Ssl {
    fn write(&mut self, buf: &[u8]) -> std::io::Result<usize> {
        loop {
            unsafe {
                let ret = SSL_write(self.0, buf.as_ptr(), buf.len() as _);
                if ret <= 0 {
                    let sslerr = SSL_get_error(self.0, ret);
                    match sslerr {
                        SSL_ERROR_WANT_WRITE => {},
                        SSL_ERROR_ZERO_RETURN => return Ok(0),
                        _ => {}
                    }
                } else {
                    return Ok(ret as _);
                }
            }
        }
    }
    
    fn flush(&mut self) -> std::io::Result<()> {
        Ok(())
    }
}

impl Drop for Ssl {
    fn drop(&mut self) {
        unsafe {
            SSL_free(self.0);
        }
    }
}