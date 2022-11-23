#version 310 es
precision mediump float;

layout(location = 0) out vec4 x_GLF_color_1_1;
struct buf0 {
  vec4 r;
};

layout(binding = 0, std140) uniform x_7_block_ubo {
  buf0 inner;
} x_7;

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
void main_1() {
  float f = 0.0f;
  vec4 v = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  f = 1.0f;
  float x_33 = f;
  float x_35 = f;
  float x_37 = f;
  float x_39 = f;
  v = vec4(sin(x_33), cos(x_35), exp2(x_37), log(x_39));
  vec4 x_42 = v;
  vec4 x_44 = x_7.inner.r;
  if ((distance(x_42, x_44) < 0.100000001f)) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f);
  }
  return;
}

struct main_out {
  vec4 x_GLF_color_1;
};

main_out tint_symbol() {
  main_1();
  main_out tint_symbol_1 = main_out(x_GLF_color);
  return tint_symbol_1;
}

void main() {
  main_out inner_result = tint_symbol();
  x_GLF_color_1_1 = inner_result.x_GLF_color_1;
  return;
}
