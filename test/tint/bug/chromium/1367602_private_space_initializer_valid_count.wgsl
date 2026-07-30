var<private> v = array<bool, 32767>();

@compute @workgroup_size(1)
fn f() {
    _ = v;
}
