#version 310 es
precision mediump float;

layout(location = 0) flat in int x_3_param_1;
layout(location = 0) out int x_4_1_1;
vec4 x_2 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
int x_3 = 0;
int x_4 = 0;
layout(binding = 0, std430) buffer S_1 {
  int field0[];
} x_5;
void main_1() {
  x_4 = 1;
  vec4 x_23 = x_2;
  int x_27 = int(x_23.x);
  int x_28 = int(x_23.y);
  int x_33 = x_3;
  x_5.field0[(x_27 + (x_28 * 8))] = x_27;
  if (((((x_27 & 1) + (x_28 & 1)) + x_33) == int(x_23.z))) {
  }
  return;
}

struct main_out {
  int x_4_1;
};

main_out tint_symbol(vec4 x_2_param, int x_3_param) {
  x_2 = x_2_param;
  x_3 = x_3_param;
  main_1();
  main_out tint_symbol_1 = main_out(x_4);
  return tint_symbol_1;
}

void main() {
  main_out inner_result = tint_symbol(gl_FragCoord, x_3_param_1);
  x_4_1_1 = inner_result.x_4_1;
  return;
}
