var<private> v = array<bool, 65535>();

@compute @workgroup_size(1)
fn f() {
    _ = v;
}
