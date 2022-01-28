SKIP: FAILED

#version 310 es
precision mediump float;

layout(location = 0) out vec4 x_GLF_color_1_1;
vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
void main_1() {
  int i = 0;
  i = 2;
  while (true) {
    i = (i + 1);
    {
      float x_35 = tint_symbol.x;
      if (((x_35 >= 0.0f) & false)) {
      } else {
        break;
      }
    }
  }
  if ((i == 3)) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  }
  return;
}

struct main_out {
  vec4 x_GLF_color_1;
};

main_out tint_symbol_1(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  main_out tint_symbol_3 = main_out(x_GLF_color);
  return tint_symbol_3;
}

void main() {
  main_out inner_result = tint_symbol_1(gl_FragCoord);
  x_GLF_color_1_1 = inner_result.x_GLF_color_1;
  return;
}
Error parsing GLSL shader:
ERROR: 0:14: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' const bool' (or there is no acceptable conversion)
ERROR: 0:14: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



