#version 310 es
precision highp float;
precision highp int;


struct SB_RW {
  int arg_0;
};

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  int tint_symbol;
} v;
layout(binding = 1, std430)
buffer tint_symbol_3_1_ssbo {
  SB_RW tint_symbol_2;
} v_1;
int atomicOr_8d96a0() {
  int res = atomicOr(v_1.tint_symbol_2.arg_0, 1);
  return res;
}
void main() {
  v.tint_symbol = atomicOr_8d96a0();
}
#version 310 es


struct SB_RW {
  int arg_0;
};

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  int tint_symbol;
} v;
layout(binding = 1, std430)
buffer tint_symbol_3_1_ssbo {
  SB_RW tint_symbol_2;
} v_1;
int atomicOr_8d96a0() {
  int res = atomicOr(v_1.tint_symbol_2.arg_0, 1);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = atomicOr_8d96a0();
}
