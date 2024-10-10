#version 310 es
precision highp float;
precision highp int;


struct buf0 {
  vec4 r;
};

struct main_out {
  vec4 x_GLF_color_1;
};

layout(binding = 0, std140)
uniform x_7_block_1_ubo {
  buf0 inner;
} v_1;
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_loc0_Output;
void main_1() {
  float f = 0.0f;
  vec4 v = vec4(0.0f);
  f = 1.0f;
  float v_2 = sin(f);
  float v_3 = cos(f);
  float v_4 = exp2(f);
  v = vec4(v_2, v_3, v_4, log(f));
  if ((distance(v, v_1.inner.r) < 0.10000000149011611938f)) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f);
  }
}
main_out tint_symbol_inner() {
  main_1();
  return main_out(x_GLF_color);
}
void main() {
  tint_symbol_loc0_Output = tint_symbol_inner().x_GLF_color_1;
}
