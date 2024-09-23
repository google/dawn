#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  float tint_symbol;
} v;
shared float arg_0;
float workgroupUniformLoad_7a857c() {
  barrier();
  float v_1 = arg_0;
  barrier();
  float res = v_1;
  return res;
}
void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    arg_0 = 0.0f;
  }
  barrier();
  v.tint_symbol = workgroupUniformLoad_7a857c();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner(gl_LocalInvocationIndex);
}
