#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_2_1_ssbo {
  int tint_symbol_1;
} v;
bool continue_execution = true;
void main() {
  if ((v.tint_symbol_1 < 0)) {
    continue_execution = false;
  }
  if (!(continue_execution)) {
    discard;
  }
}
