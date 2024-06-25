#version 310 es

shared uint arg_0;
uint tint_workgroupUniformLoad_arg_0() {
  barrier();
  uint result = arg_0;
  barrier();
  return result;
}

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    arg_0 = 0u;
  }
  barrier();
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

uint workgroupUniformLoad_37307c() {
  uint res = tint_workgroupUniformLoad_arg_0();
  return res;
}

void compute_main(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  prevent_dce.inner = workgroupUniformLoad_37307c();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
