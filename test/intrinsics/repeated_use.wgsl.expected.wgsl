[[stage(compute), workgroup_size(1)]]
fn main() {
  _ = isNormal(vec4<f32>());
  _ = isNormal(vec4<f32>(1.0));
  _ = isNormal(vec4<f32>(1.0, 2.0, 3.0, 4.0));
  _ = isNormal(vec3<f32>());
  _ = isNormal(vec3<f32>(1.0));
  _ = isNormal(vec3<f32>(1.0, 2.0, 3.0));
  _ = isNormal(vec2<f32>());
  _ = isNormal(vec2<f32>(1.0));
  _ = isNormal(vec2<f32>(1.0, 2.0));
  _ = isNormal(1.0);
  _ = isNormal(2.0);
  _ = isNormal(3.0);
}
