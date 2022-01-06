SKIP: FAILED

#version 310 es
precision mediump float;

struct tint_padded_array_element {
  int el;
};
struct buf0 {
  tint_padded_array_element x_GLF_uniform_int_values[2];
};

layout (binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_int_values[2];
} x_6;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  mat2 m = mat2(0.0f, 0.0f, 0.0f, 0.0f);
  int x_29 = x_6.x_GLF_uniform_int_values[0].el;
  float x_30 = float(x_29);
  m = transpose(transpose(mat2(vec2(x_30, 0.0f), vec2(0.0f, x_30))));
  mat2 x_36 = m;
  int x_38 = x_6.x_GLF_uniform_int_values[0].el;
  float x_39 = float(x_38);
  mat2 x_42 = mat2(vec2(x_39, 0.0f), vec2(0.0f, x_39));
  if ((all(equal(x_36[0u], x_42[0u])) & all(equal(x_36[1u], x_42[1u])))) {
    int x_56 = x_6.x_GLF_uniform_int_values[0].el;
    int x_59 = x_6.x_GLF_uniform_int_values[1].el;
    int x_62 = x_6.x_GLF_uniform_int_values[1].el;
    int x_65 = x_6.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4(float(x_56), float(x_59), float(x_62), float(x_65));
  } else {
    int x_69 = x_6.x_GLF_uniform_int_values[1].el;
    float x_70 = float(x_69);
    x_GLF_color = vec4(x_70, x_70, x_70, x_70);
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
ERROR: 0:25: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' global bool' and a right operand of type ' global bool' (or there is no acceptable conversion)
ERROR: 0:25: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



