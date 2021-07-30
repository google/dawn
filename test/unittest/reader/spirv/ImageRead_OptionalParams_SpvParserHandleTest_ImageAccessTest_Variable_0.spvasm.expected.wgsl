warning: use of deprecated intrinsic
[[group(2), binding(1)]] var x_20 : texture_storage_2d<rgba32float, read>;

fn main_1() {
  let f1 : f32 = 1.0;
  let vf12 : vec2<f32> = vec2<f32>(1.0, 2.0);
  let vf123 : vec3<f32> = vec3<f32>(1.0, 2.0, 3.0);
  let vf1234 : vec4<f32> = vec4<f32>(1.0, 2.0, 3.0, 4.0);
  let i1 : i32 = 1;
  let vi12 : vec2<i32> = vec2<i32>(1, 2);
  let vi123 : vec3<i32> = vec3<i32>(1, 2, 3);
  let vi1234 : vec4<i32> = vec4<i32>(1, 2, 3, 4);
  let u1 : u32 = 1u;
  let vu12 : vec2<u32> = vec2<u32>(1u, 2u);
  let vu123 : vec3<u32> = vec3<u32>(1u, 2u, 3u);
  let vu1234 : vec4<u32> = vec4<u32>(1u, 2u, 3u, 4u);
  let offsets2d : vec2<i32> = vec2<i32>(3, 4);
  let x_99 : vec4<f32> = textureLoad(x_20, vi12);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
