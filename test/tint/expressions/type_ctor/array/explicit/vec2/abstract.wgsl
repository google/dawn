var<private> arr = array<vec2<f32>, 2>(vec2(1), vec2(2));

@compute @workgroup_size(1)
fn f() {
    var v = arr;
}
