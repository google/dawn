SKIP: FAILED

#version 310 es
precision mediump float;

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  vec2 a = vec2(0.0f, 0.0f);
  vec2 b = vec2(0.0f, 0.0f);
  a = vec2(1.0f, 1.0f);
  float x_25 = a.x;
  a.x = (x_25 + 0.5f);
  b = frac(a);
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
ERROR: 0:12: 'frac' : no matching overloaded function found 
ERROR: 0:12: 'assign' :  cannot convert from ' const float' to ' temp mediump 2-component vector of float'
ERROR: 0:12: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



