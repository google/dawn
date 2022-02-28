#version 310 es
precision mediump float;

layout(location = 0) out vec4 x_GLF_color_1_1;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
void main_1() {
  vec2 a = vec2(0.0f, 0.0f);
  vec2 b = vec2(0.0f, 0.0f);
  a = vec2(1.0f, 1.0f);
  float x_25 = a.x;
  a.x = (x_25 + 0.5f);
  b = fract(a);
  float x_31 = b.x;
  if ((x_31 == 0.5f)) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
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
