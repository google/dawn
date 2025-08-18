@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  var a : f32;
  var b : f32;
  a = 42.0f;
  b = radians(a);
}
