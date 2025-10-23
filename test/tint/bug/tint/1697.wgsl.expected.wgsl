override O = 0;

@compute @workgroup_size(1)
fn f() {
  const smaller_than_any_f32 = 1e-50;
  var v = vec2(0)[i32(((vec2(smaller_than_any_f32)[O] * 1000000000000000013287555072.0) * 1000000000000000013287555072.0))];
}
