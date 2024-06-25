#version 310 es

shared float arg_0;
float tint_workgroupUniformLoad_arg_0() {
  barrier();
  float result = arg_0;
  barrier();
  return result;
}

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    arg_0 = 0.0f;
  }
  barrier();
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

float workgroupUniformLoad_7a857c() {
  float res = tint_workgroupUniformLoad_arg_0();
  return res;
}

void compute_main(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  prevent_dce.inner = workgroupUniformLoad_7a857c();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
