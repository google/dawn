SKIP: FAILED

#version 310 es
precision mediump float;

struct tint_padded_array_element {
  int el;
};
struct buf0 {
  tint_padded_array_element x_GLF_uniform_int_values[2];
};
struct tint_padded_array_element_1 {
  float el;
};
struct buf1 {
  tint_padded_array_element_1 x_GLF_uniform_float_values[2];
};

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout (binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_int_values[2];
} x_5;
layout (binding = 1) uniform buf1_1 {
  tint_padded_array_element_1 x_GLF_uniform_float_values[2];
} x_8;

void main_1() {
  float a = 0.0f;
  int x_10 = x_5.x_GLF_uniform_int_values[0].el;
  int x_11 = x_5.x_GLF_uniform_int_values[1].el;
  int x_12 = x_5.x_GLF_uniform_int_values[1].el;
  int x_13 = x_5.x_GLF_uniform_int_values[0].el;
  x_GLF_color = vec4(float(x_10), float(x_11), float(x_12), float(x_13));
  float x_45 = x_8.x_GLF_uniform_float_values[1].el;
  a = (x_45 % uintBitsToFloat(0x7f800000u));
  float x_47 = a;
  float x_49 = x_8.x_GLF_uniform_float_values[0].el;
  if ((x_47 != x_49)) {
    float x_54 = x_8.x_GLF_uniform_float_values[0].el;
    x_GLF_color.y = x_54;
  }
  return;
}

struct main_out {
  vec4 x_GLF_color_1;
};
struct tint_symbol_1 {
  vec4 x_GLF_color_1;
};

main_out tint_symbol_inner() {
  main_1();
  main_out tint_symbol_2 = main_out(x_GLF_color);
  return tint_symbol_2;
}

tint_symbol_1 tint_symbol() {
  main_out inner_result = tint_symbol_inner();
  tint_symbol_1 wrapper_result = tint_symbol_1(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
out vec4 x_GLF_color_1;
void main() {
  tint_symbol_1 outputs;
  outputs = tint_symbol();
  x_GLF_color_1 = outputs.x_GLF_color_1;
}


Error parsing GLSL shader:
ERROR: 0:33: '%' :  wrong operand types: no operation '%' exists that takes a left-hand operand of type ' temp mediump float' and a right operand of type ' global highp float' (or there is no acceptable conversion)
ERROR: 0:33: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



