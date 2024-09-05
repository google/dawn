#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_3_1_ssbo {
  int tint_symbol_2;
} v;
layout(binding = 1, std430)
buffer tint_symbol_5_1_ssbo {
  float tint_symbol_4;
} v_1;
bool continue_execution = true;
void main() {
  if ((v.tint_symbol_2 < 0)) {
    continue_execution = false;
  }
  float v_2 = dFdx(1.0f);
  if (continue_execution) {
    v_1.tint_symbol_4 = v_2;
  }
  if (!(continue_execution)) {
    discard;
  }
}
