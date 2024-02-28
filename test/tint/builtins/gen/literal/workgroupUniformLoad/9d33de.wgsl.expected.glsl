#version 310 es

shared int arg_0;
void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    arg_0 = 0;
  }
  barrier();
}

int tint_workgroupUniformLoad_arg_0() {
  barrier();
  int result = arg_0;
  barrier();
  return result;
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

void workgroupUniformLoad_9d33de() {
  int res = tint_workgroupUniformLoad_arg_0();
  prevent_dce.inner = res;
}

void compute_main(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  workgroupUniformLoad_9d33de();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
