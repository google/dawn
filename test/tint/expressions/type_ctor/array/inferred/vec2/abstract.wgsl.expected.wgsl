var<private> arr = array(vec2<f32>(1), vec2<f32>(2));

@compute @workgroup_size(1)
fn f() {
  var v = arr;
}
