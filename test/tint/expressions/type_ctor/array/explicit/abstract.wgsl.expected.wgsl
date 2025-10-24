var<private> arr = array<f32, 2>(1, 2);

@compute @workgroup_size(1)
fn f() {
  var v = arr;
}
