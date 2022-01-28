SKIP: FAILED

#version 310 es
precision mediump float;

layout(location = 0) out vec4 x_GLF_color_1_1;
struct tint_padded_array_element {
  float el;
};

struct buf0 {
  tint_padded_array_element x_GLF_uniform_float_values[2];
};

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout(binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_float_values[2];
} x_8;

void main_1() {
  float a = 0.0f;
  float b = 0.0f;
  float c = 0.0f;
  a = -1.0f;
  b = 1.700000048f;
  c = pow(a, b);
  float x_30 = c;
  x_GLF_color = vec4(x_30, x_30, x_30, x_30);
  if (((a == -1.0f) & (b == 1.700000048f))) {
    float x_41 = x_8.x_GLF_uniform_float_values[0].el;
    float x_43 = x_8.x_GLF_uniform_float_values[1].el;
    float x_45 = x_8.x_GLF_uniform_float_values[1].el;
    float x_47 = x_8.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4(x_41, x_43, x_45, x_47);
  } else {
    float x_50 = x_8.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4(x_50, x_50, x_50, x_50);
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
Error parsing GLSL shader:
ERROR: 0:27: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:27: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



