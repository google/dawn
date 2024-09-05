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
  if ((v_1.tint_symbol_4 < 0.0f)) {
    int i = 0;
    {
      while(true) {
        float v_3 = v_1.tint_symbol_4;
        if ((v_3 > float(i))) {
          float v_4 = float(i);
          if (continue_execution) {
            v_1.tint_symbol_4 = v_4;
          }
          if (!(continue_execution)) {
            discard;
          }
          return;
        }
        {
          i = (i + 1);
          if ((i == 5)) { break; }
        }
        continue;
      }
    }
    if (!(continue_execution)) {
      discard;
    }
    return;
  }
  if (!(continue_execution)) {
    discard;
  }
}
