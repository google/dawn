#version 310 es
precision mediump float;

layout(location = 0) out vec4 x_GLF_color_1_1;
struct buf0 {
  float two;
};

vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout(binding = 0) uniform buf0_1 {
  float two;
} x_8;

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
void main_1() {
  float a = 0.0f;
  float b = 0.0f;
  float x_33 = tint_symbol.x;
  a = dFdx(cos(x_33));
  float x_37 = x_8.two;
  b = mix(2.0f, x_37, a);
  if (bool(uint((b >= 1.899999976f)) & uint((b <= 2.099999905f)))) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  }
  return;
}

struct main_out {
  vec4 x_GLF_color_1;
};

main_out tint_symbol_1(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  main_out tint_symbol_3 = main_out(x_GLF_color);
  return tint_symbol_3;
}

void main() {
  main_out inner_result = tint_symbol_1(gl_FragCoord);
  x_GLF_color_1_1 = inner_result.x_GLF_color_1;
  return;
}
