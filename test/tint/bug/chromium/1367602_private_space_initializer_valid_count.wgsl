var<private> v = array<bool, 8192>();

@compute @workgroup_size(1)
fn f() {
    _ = v;
}
