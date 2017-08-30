mod ngfx_common;

#[cfg(any(unix, windows))]
mod ngfx_vulkan;
#[cfg(any(macos, ios))]
mod ngfx_metal;

