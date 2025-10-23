fn f() -> f32 {
  var v = vec3<f32>(1.0, 2.0, 3.0);
  return v[1];
}

@compute @workgroup_size(1)
fn main() {
  _ = f();
}
