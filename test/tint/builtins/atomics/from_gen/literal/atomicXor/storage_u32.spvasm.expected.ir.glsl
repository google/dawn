#version 310 es
precision highp float;
precision highp int;


struct SB_RW_atomic {
  uint arg_0;
};

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  SB_RW_atomic tint_symbol;
} v;
void atomicXor_54510e() {
  uint res = 0u;
  uint x_9 = atomicXor(v.tint_symbol.arg_0, 1u);
  res = x_9;
}
void fragment_main_1() {
  atomicXor_54510e();
}
void main() {
  fragment_main_1();
}
#version 310 es


struct SB_RW_atomic {
  uint arg_0;
};

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  SB_RW_atomic tint_symbol;
} v;
void atomicXor_54510e() {
  uint res = 0u;
  uint x_9 = atomicXor(v.tint_symbol.arg_0, 1u);
  res = x_9;
}
void compute_main_1() {
  atomicXor_54510e();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_1();
}
