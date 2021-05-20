struct S {
};

var<private> bool_var : bool = bool();

let bool_let : bool = bool();

var<private> i32_var : i32 = i32();

let i32_let : i32 = i32();

var<private> u32_var : u32 = u32();

let u32_let : u32 = u32();

var<private> f32_var : f32 = f32();

let f32_let : f32 = f32();

var<private> v2i32_var : vec2<i32> = vec2<i32>();

let v2i32_let : vec2<i32> = vec2<i32>();

var<private> v3u32_var : vec3<u32> = vec3<u32>();

let v3u32_let : vec3<u32> = vec3<u32>();

var<private> v4f32_var : vec4<f32> = vec4<f32>();

let v4f32_let : vec4<f32> = vec4<f32>();

var<private> m2x3_var : mat2x3<f32> = mat2x3<f32>();

let m3x4_let : mat3x4<f32> = mat3x4<f32>();

var<private> arr_var : array<f32, 4> = array<f32, 4>();

let arr_let : array<f32, 4> = array<f32, 4>();

var<private> struct_var : S = S();

let struct_let : S = S();

[[stage(compute)]]
fn main() {
}
