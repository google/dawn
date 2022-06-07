struct S {
  a : f32,
}

fn foo(param_bool : bool, param_i32 : i32, param_u32 : u32, param_f32 : f32, param_v2i32 : vec2<i32>, param_v3u32 : vec3<u32>, param_v4f32 : vec4<f32>, param_m2x3 : mat2x3<f32>, param_arr : array<f32, 4>, param_struct : S, param_ptr_f32 : ptr<function, f32>, param_ptr_vec : ptr<function, vec4<f32>>, param_ptr_arr : ptr<function, array<f32, 4>>) {
}

@compute @workgroup_size(1)
fn main() {
}
