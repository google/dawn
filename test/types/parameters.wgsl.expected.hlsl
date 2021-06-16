struct S {
};
struct tint_array_wrapper {
  float arr[4];
};

void foo(bool param_bool, int param_i32, uint param_u32, float param_f32, int2 param_v2i32, uint3 param_v3u32, float4 param_v4f32, float2x3 param_m2x3, tint_array_wrapper param_arr, S param_struct, inout float param_ptr_f32, inout float4 param_ptr_vec, inout tint_array_wrapper param_ptr_arr) {
}

[numthreads(1, 1, 1)]
void main() {
  return;
}
