var<private> t : u32;
fn m() -> u32 {
    t = 1u;
    return u32(t);
}

@compute @workgroup_size(1)
fn f() {
    var v : i32 = i32(m());
}
