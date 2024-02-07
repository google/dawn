#version 310 es
precision highp float;
precision highp int;

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
  v = vec4(sin(f), cos(f), exp2(f), log(f));
  if ((distance(v, x_7.inner.r) < 0.10000000149011611938f)) {
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
