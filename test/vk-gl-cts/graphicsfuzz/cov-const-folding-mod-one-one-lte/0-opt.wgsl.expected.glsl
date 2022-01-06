SKIP: FAILED

#version 310 es
precision mediump float;

struct tint_padded_array_element {
  int el;
};
struct buf0 {
  tint_padded_array_element x_GLF_uniform_int_values[2];
};

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout (binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_int_values[2];
} x_5;

void main_1() {
  if (((1.0f % 1.0f) <= 0.01f)) {
    int x_29 = x_5.x_GLF_uniform_int_values[0].el;
    int x_32 = x_5.x_GLF_uniform_int_values[0].el;
    int x_35 = x_5.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4(1.0f, float(x_29), float(x_32), float(x_35));
  } else {
    int x_39 = x_5.x_GLF_uniform_int_values[0].el;
    float x_40 = float(x_39);
    x_GLF_color = vec4(x_40, x_40, x_40, x_40);
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
ERROR: 0:17: '%' :  wrong operand types: no operation '%' exists that takes a left-hand operand of type ' const float' and a right operand of type ' const float' (or there is no acceptable conversion)
ERROR: 0:17: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



