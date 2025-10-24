var<private> arr = array<mat2x2<f32>, 2>(mat2x2<f32>(1, 2, 3, 4), mat2x2<f32>(5, 6, 7, 8));

@compute @workgroup_size(1)
fn f() {
  var v = arr;
}
