var<private> t : i32;
fn m() -> i32 {
    t = 1i;
    return i32(t);
}

@compute @workgroup_size(1)
fn f() {
    var v : f32 = f32(m());
}
