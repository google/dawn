#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

shared float16_t arg_0;
float16_t tint_workgroupUniformLoad_arg_0() {
  barrier();
  float16_t result = arg_0;
  barrier();
  return result;
}

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    arg_0 = 0.0hf;
  }
  barrier();
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float16_t inner;
} prevent_dce;

float16_t workgroupUniformLoad_e07d08() {
  float16_t res = tint_workgroupUniformLoad_arg_0();
  return res;
}

void compute_main(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  prevent_dce.inner = workgroupUniformLoad_e07d08();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
