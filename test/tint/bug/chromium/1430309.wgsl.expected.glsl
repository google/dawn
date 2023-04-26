#version 310 es
precision highp float;

struct frexp_result_f32_1 {
  float fract;
  int exp;
};


layout(location = 0) out vec4 value;
struct frexp_result_f32 {
  float f;
};

frexp_result_f32 a = frexp_result_f32(0.0f);
frexp_result_f32_1 b = frexp_result_f32_1(0.5f, 1);
vec4 tint_symbol() {
  return vec4(a.f, b.fract, 0.0f, 0.0f);
}

void main() {
  vec4 inner_result = tint_symbol();
  value = inner_result;
  return;
}
