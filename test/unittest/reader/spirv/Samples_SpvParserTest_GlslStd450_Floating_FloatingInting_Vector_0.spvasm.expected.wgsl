fn main_1() {
  let u1 : u32 = 10u;
  let u2 : u32 = 15u;
  let u3 : u32 = 20u;
  let i1 : i32 = 30;
  let i2 : i32 = 35;
  let i3 : i32 = 40;
  let f1 : f32 = 50.0;
  let f2 : f32 = 60.0;
  let f3 : f32 = 70.0;
  let v2u1 : vec2<u32> = vec2<u32>(10u, 20u);
  let v2u2 : vec2<u32> = vec2<u32>(20u, 10u);
  let v2u3 : vec2<u32> = vec2<u32>(15u, 15u);
  let v2i1 : vec2<i32> = vec2<i32>(30, 40);
  let v2i2 : vec2<i32> = vec2<i32>(40, 30);
  let v2i3 : vec2<i32> = vec2<i32>(35, 35);
  let v2f1 : vec2<f32> = vec2<f32>(50.0, 60.0);
  let v2f2 : vec2<f32> = vec2<f32>(60.0, 50.0);
  let v2f3 : vec2<f32> = vec2<f32>(70.0, 70.0);
  let v3f1 : vec3<f32> = vec3<f32>(50.0, 60.0, 70.0);
  let v3f2 : vec3<f32> = vec3<f32>(60.0, 70.0, 50.0);
  let v4f1 : vec4<f32> = vec4<f32>(50.0, 50.0, 50.0, 50.0);
  let v4f2 : vec4<f32> = v4f1;
  let x_1 : vec2<f32> = ldexp(v2f1, v2i1);
  return;
}

[[stage(compute), workgroup_size(1, 1, 1)]]
fn main() {
  main_1();
}
