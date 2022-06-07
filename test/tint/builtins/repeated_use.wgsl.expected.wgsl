@compute @workgroup_size(1)
fn main() {
  _ = degrees(vec4<f32>());
  _ = degrees(vec4<f32>(1.0));
  _ = degrees(vec4<f32>(1.0, 2.0, 3.0, 4.0));
  _ = degrees(vec3<f32>());
  _ = degrees(vec3<f32>(1.0));
  _ = degrees(vec3<f32>(1.0, 2.0, 3.0));
  _ = degrees(vec2<f32>());
  _ = degrees(vec2<f32>(1.0));
  _ = degrees(vec2<f32>(1.0, 2.0));
  _ = degrees(1.0);
  _ = degrees(2.0);
  _ = degrees(3.0);
}
