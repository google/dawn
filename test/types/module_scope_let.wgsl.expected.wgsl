struct S {
  a : f32;
};

let bool_let : bool = bool();

let i32_let : i32 = i32();

let u32_let : u32 = u32();

let f32_let : f32 = f32();

let v2i32_let : vec2<i32> = vec2<i32>();

let v3u32_let : vec3<u32> = vec3<u32>();

let v4f32_let : vec4<f32> = vec4<f32>();

let m3x4_let : mat3x4<f32> = mat3x4<f32>();

let arr_let : array<f32, 4> = array<f32, 4>();

let struct_let : S = S();

[[stage(compute), workgroup_size(1)]]
fn main() {
}
