#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  float16_t tint_symbol;
} v;
shared float16_t arg_0;
float16_t workgroupUniformLoad_e07d08() {
  barrier();
  float16_t v_1 = arg_0;
  barrier();
  float16_t res = v_1;
  return res;
}
void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    arg_0 = 0.0hf;
  }
  barrier();
  v.tint_symbol = workgroupUniformLoad_e07d08();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner(gl_LocalInvocationIndex);
}
