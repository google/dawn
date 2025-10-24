var<private> arr = array<f32, 2>(1f, 2f);

@compute @workgroup_size(1)
fn f() {
    var v = arr;
}
