SKIP: FAILED

#version 310 es
precision mediump float;

layout(location = 0) out vec4 x_GLF_color_1_1;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
void main_1() {
  vec4 v = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  float dist1 = 0.0f;
  float dist2 = 0.0f;
  v = vec4(1.0f, 2.0f, 3.0f, 4.0f);
  dist1 = distance(tanh(v), (sinh(v) / cosh(v)));
  dist2 = distance(tanh(v), vec4(0.761590004f, 0.964030027f, 0.995050013f, 0.999329984f));
  if (((dist1 < 0.100000001f) & (dist2 < 0.100000001f))) {
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
ERROR: 0:13: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:13: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



