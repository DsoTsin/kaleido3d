use std::env;
use std::path::PathBuf;
use std::process::Command;

extern crate cbindgen;
use cbindgen::{Config, Language, ExportConfig};

fn main() {
    let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap();

    let package_name = env::var("CARGO_PKG_NAME").unwrap();
    let output_file = target_dir()
        .join(format!("{}.h", package_name))
        .display()
        .to_string();

    let config = Config {
        namespace: Some(String::from("ffi")),
        pragma_once: true,
        language: Language::C,
        cpp_compat: true,
        export: ExportConfig {
            exclude: vec![
                "BIOMethod".into(), 
                "BIO".into(), 
                "SSLMethod".into(), 
                "SSLContext".into(), 
                "X509".into(), 
                "RSA".into(), 
                "SSL".into(),
                "BIO_s_socket".into(),
                "BIO_new".into(),
                "BIO_int_ctrl".into(),
                "BIO_free".into(),
                "TLS_method".into(),
                "SSL_CTX_new".into(),
                "SSL_CTX_free".into(),
                "BIO_new_mem_buf".into(),
                "PEM_read_bio_X509".into(),
                "SSL_CTX_use_certificate".into(),
                "X509_free".into(),
                "PEM_read_bio_RSAPrivateKey".into(),
                "SSL_CTX_use_RSAPrivateKey".into(),
                "RSA_free".into(),
                "SSL_new".into(),
                "SSL_free".into(),
                "SSL_set_connect_state".into(),
                "SSL_set_verify".into(),
                "SSL_set_bio".into(),
                "SSL_do_handshake".into(),
                "SSL_get_error".into(),
                "SSL_get_version".into(),
                "SSL_shutdown".into(),
                "SSL_write".into(),
                "SSL_pending".into(),
                "SSL_read".into()
            ],
            ..Default::default()  
        },
        ..Default::default()
    };

    cbindgen::generate_with_config(&crate_dir, config)
      .unwrap()
      .write_to_file(&output_file);
}

/// Find the location of the `target/` directory. Note that this may be 
/// overridden by `cmake`, so we also need to check the `CARGO_TARGET_DIR` 
/// variable.
fn target_dir() -> PathBuf {
    if let Ok(target) = env::var("CARGO_TARGET_DIR") {
        PathBuf::from(target)
    } else {
        PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap()).join("target")
    }
}