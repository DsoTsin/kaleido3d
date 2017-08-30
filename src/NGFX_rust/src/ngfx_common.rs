#![feature(lang_items)]

extern crate libc;

pub enum Backend {}

#[link(name = "Kaleido3D.Core")]
extern {
    fn k3d_malloc(SzToAlloc: libc::size_t) -> *mut u8;
}

#[link(name = "Kaleido3D.Core")]
extern {
    fn k3d_free(Ptr: *mut libc::c_void, SzToFree: libc::size_t);
}

#[lang = "exchange_malloc"]
unsafe fn allocate(size: usize, _align: usize) -> *mut u8 {
    let p = libc::malloc(size as libc::size_t) as *mut u8;

    // malloc failed
    if p as usize == 0 {
    }

    p
}

#[lang = "exchange_free"]
unsafe fn deallocate(ptr: *mut u8, _size: usize, _align: usize) {
    libc::free(ptr as *mut libc::c_void)
}


