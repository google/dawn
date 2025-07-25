#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  int inner;
} v;
shared bool arg_0;
int workgroupUniformLoad_b75d53() {
  barrier();
  bool v_1 = arg_0;
  barrier();
  bool res = v_1;
  return mix(0, 1, (res == false));
}
void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    arg_0 = false;
  }
  barrier();
  v.inner = workgroupUniformLoad_b75d53();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner(gl_LocalInvocationIndex);
}
