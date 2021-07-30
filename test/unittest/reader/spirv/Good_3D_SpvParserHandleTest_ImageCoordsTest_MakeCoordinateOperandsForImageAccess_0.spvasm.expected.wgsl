[[group(0), binding(0)]] var x_10 : sampler;

[[group(2), binding(1)]] var x_20 : texture_3d<f32>;

[[group(0), binding(1)]] var x_30 : sampler;

fn main_1() {
  var float_var : f32;
  let i1 : i32 = 1;
  let vi12 : vec2<i32> = vec2<i32>(1, 2);
  let vi123 : vec3<i32> = vec3<i32>(1, 2, 3);
  let vi1234 : vec4<i32> = vec4<i32>(1, 2, 3, 4);
  let u1 : u32 = 1u;
  let vu12 : vec2<u32> = vec2<u32>(1u, 2u);
  let vu123 : vec3<u32> = vec3<u32>(1u, 2u, 3u);
  let vu1234 : vec4<u32> = vec4<u32>(1u, 2u, 3u, 4u);
  let f1 : f32 = 1.0;
  let vf12 : vec2<f32> = vec2<f32>(1.0, 2.0);
  let vf123 : vec3<f32> = vec3<f32>(1.0, 2.0, 3.0);
  let vf1234 : vec4<f32> = vec4<f32>(1.0, 2.0, 3.0, 4.0);
  let x_73 : vec4<f32> = textureSample(x_20, x_10, vf123);
  let x_1000 : u32 = 0u;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
