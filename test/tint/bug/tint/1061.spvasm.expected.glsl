#version 310 es
precision mediump float;

layout(location = 0) out vec4 x_GLF_color_1_1;
layout(binding = 0, std140) uniform buf0_ubo {
  vec4 ref;
} x_7;

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
void main_1() {
  float f = 0.0f;
  vec4 v = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  f = determinant(mat3(vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)));
  v = vec4(sin(f), cos(f), exp2(f), log(f));
  vec4 x_42 = v;
  vec4 x_44 = x_7.ref;
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
