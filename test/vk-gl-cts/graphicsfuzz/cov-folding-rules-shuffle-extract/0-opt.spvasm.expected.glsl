SKIP: FAILED

#version 310 es
precision mediump float;

layout(location = 0) out vec4 x_GLF_color_1_1;
struct buf0 {
  vec2 injectionSwitch;
};

layout(binding = 0) uniform buf0_1 {
  vec2 injectionSwitch;
} x_5;

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
void main_1() {
  vec2 x_29 = x_5.injectionSwitch;
  if (((bvec2(false, false) ? vec2(1.0f, 1.0f) : x_29).x == 0.0f)) {
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
ERROR: 0:16: '' : boolean expression expected 
ERROR: 0:16: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



