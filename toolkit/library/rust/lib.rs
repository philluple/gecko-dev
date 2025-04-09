extern crate pdslib;

#[no_mangle]
pub extern "C" fn pdslib_dummy_link() {
    let _ = pdslib::some_public_function();
}
