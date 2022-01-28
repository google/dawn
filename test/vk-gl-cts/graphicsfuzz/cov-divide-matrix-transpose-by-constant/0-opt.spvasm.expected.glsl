SKIP: FAILED

#version 310 es
precision mediump float;

layout(location = 0) out vec4 x_GLF_color_1_1;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
void main_1() {
  mat2 m = mat2(0.0f, 0.0f, 0.0f, 0.0f);
  m = (transpose(mat2(vec2(1.0f, 2.0f), vec2(3.0f, 4.0f))) * (1.0f / 2.0f));
  mat2 x_33 = m;
  if ((all(equal(x_33[0u], mat2(vec2(0.5f, 1.5f), vec2(1.0f, 2.0f))[0u])) & all(equal(x_33[1u], mat2(vec2(0.5f, 1.5f), vec2(1.0f, 2.0f))[1u])))) {
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
Error parsing GLSL shader:
ERROR: 0:10: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' global bool' and a right operand of type ' global bool' (or there is no acceptable conversion)
ERROR: 0:10: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



