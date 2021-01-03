use std::{error, fmt, io};
use std::convert::From;

#[derive(Debug)]
pub struct Error {
    inner: Box<ErrorImpl>,
}

#[derive(Debug)]
pub(crate) struct ErrorImpl {
    kind: ErrorKind,
}

#[derive(Debug)]
pub enum UsbMuxdError {
    BadCommand,
    BadDev,
    ConnRefused,
    BadVersion,
    Unknown,
}


impl From<i32> for UsbMuxdError {
    fn from(code: i32) -> Self {
        match code {
            1 => UsbMuxdError::BadCommand,
            2 => UsbMuxdError::BadDev,
            3 => UsbMuxdError::ConnRefused,
            6 => UsbMuxdError::BadVersion,
            _ => UsbMuxdError::Unknown
        }
    }
}

#[derive(Debug)]
pub(crate) enum ErrorKind {
    UnexpectedEof,
    UsbmuxdResponsePayloadEmpty,
    UnknownObjectType(u8),
    Plist(plist::Error),
    BinError(bincode::Error),
    Io(io::Error),
    Serde(String),
    UsbmuxdErr(UsbMuxdError),
}

impl Error {
    /// Returns true if this error was caused by a failure to read or write bytes on an IO stream.
    pub fn is_io(&self) -> bool {
        self.as_io().is_some()
    }

    /// Returns true if this error was caused by prematurely reaching the end of the input data.
    pub fn is_eof(&self) -> bool {
        if let ErrorKind::UnexpectedEof = self.inner.kind {
            true
        } else {
            false
        }
    }

    /// Returns the underlying error if it was caused by a failure to read or write bytes on an IO
    /// stream.
    pub fn as_io(&self) -> Option<&io::Error> {
        if let ErrorKind::Io(err) = &self.inner.kind {
            Some(err)
        } else {
            None
        }
    }

    /// Returns the underlying error if it was caused by a failure to read or write bytes on an IO
    /// stream or `self` if it was not.
    pub fn into_io(self) -> Result<io::Error, Self> {
        if let ErrorKind::Io(err) = self.inner.kind {
            Ok(err)
        } else {
            Err(self)
        }
    }
}

impl error::Error for Error {
    fn source(&self) -> Option<&(dyn std::error::Error + 'static)> {
        match &self.inner.kind {
            ErrorKind::Io(err) => Some(err),
            _ => None,
        }
    }
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        fmt::Debug::fmt(&self.inner.kind, f)
    }
}

impl ErrorKind {
    pub fn with_plist(perr: plist::Error) -> Error {
        Error {
            inner: Box::new(ErrorImpl {
                kind: ErrorKind::Plist(perr)
            }),
        }
    }
    pub fn with_err(err: ErrorKind) -> Error {
        Error {
            inner: Box::new(ErrorImpl {
                kind: err
            }),
        }
    }
    pub fn with_bin(err: bincode::Error) -> Error {
        Error {
            inner: Box::new(ErrorImpl {
                kind: ErrorKind::BinError(err)
            }),
        }
    }
    pub fn with_usbmuxd(err: UsbMuxdError) -> Error {
        Error {
            inner: Box::new(ErrorImpl{
                kind: ErrorKind::UsbmuxdErr(err)
            })
        }
    }
}
