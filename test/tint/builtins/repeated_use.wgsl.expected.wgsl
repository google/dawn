@compute @workgroup_size(1)
fn main() {
  let a = degrees(vec4<f32>());
  let b = degrees(vec4<f32>(1.0));
  let c = degrees(vec4<f32>(1.0, 2.0, 3.0, 4.0));
  let d = degrees(vec3<f32>());
  let e = degrees(vec3<f32>(1.0));
  let f = degrees(vec3<f32>(1.0, 2.0, 3.0));
  let g = degrees(vec2<f32>());
  let h = degrees(vec2<f32>(1.0));
  let i = degrees(vec2<f32>(1.0, 2.0));
  let j = degrees(1.0);
  let k = degrees(2.0);
  let l = degrees(3.0);
}
