#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  int tint_symbol;
} v;
shared int arg_0;
int workgroupUniformLoad_9d33de() {
  barrier();
  int v_1 = arg_0;
  barrier();
  int res = v_1;
  return res;
}
void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    arg_0 = 0;
  }
  barrier();
  v.tint_symbol = workgroupUniformLoad_9d33de();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner(gl_LocalInvocationIndex);
}
