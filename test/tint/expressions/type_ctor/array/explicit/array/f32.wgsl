var<private> arr = array<array<f32, 2>, 2>(array<f32, 2>(1f, 2f),
                                           array<f32, 2>(3f, 4f));

@compute @workgroup_size(1)
fn f() {
    var v = arr;
}
