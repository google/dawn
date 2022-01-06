SKIP: FAILED

#version 310 es
precision mediump float;

struct buf0 {
  int zero;
};

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout (binding = 0) uniform buf0_1 {
  int zero;
} x_5;

void main_1() {
  int x_9[1] = int[1](0);
  int x_10_phi = 0;
  int x_6 = x_9[0u];
  while (true) {
    x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    int x_7 = x_5.zero;
    int x_8 = x_9[x_7];
    if ((x_8 == x_6)) {
      x_10_phi = 1;
      break;
    }
    x_10_phi = 2;
    break;
  }
  int x_10 = x_10_phi;
  if (((x_10 == 1) | (x_10 == 2))) {
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
ERROR: 0:29: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:29: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



