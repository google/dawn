var<private> arr = array<array<f32, 2>, 2>(array<f32, 2>(1, 2),
                                           array<f32, 2>(3, 4));

@compute @workgroup_size(1)
fn f() {
    var v = arr;
}
