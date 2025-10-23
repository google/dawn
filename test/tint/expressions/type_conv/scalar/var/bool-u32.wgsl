var<private> u = bool(true);

@compute @workgroup_size(1)
fn f() {
    let v : u32 = u32(u);
}
