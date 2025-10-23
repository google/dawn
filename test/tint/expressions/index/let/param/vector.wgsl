fn f(i : i32) -> f32 {
  let v = vec3<f32>(1.0, 2.0, 3.0);
  return v[i];
}

@compute @workgroup_size(1)
fn main() {
     _ = f(1);
}
