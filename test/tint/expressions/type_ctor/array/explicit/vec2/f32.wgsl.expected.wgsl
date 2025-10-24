var<private> arr = array<vec2<f32>, 2>(vec2<f32>(1.0f), vec2<f32>(2.0f));

@compute @workgroup_size(1)
fn f() {
  var v = arr;
}
