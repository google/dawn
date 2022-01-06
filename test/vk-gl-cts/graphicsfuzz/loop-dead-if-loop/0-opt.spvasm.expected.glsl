SKIP: FAILED

#version 310 es
precision mediump float;

struct buf0 {
  vec2 injectionSwitch;
};

layout (binding = 0) uniform buf0_1 {
  vec2 injectionSwitch;
} x_6;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int k = 0;
  int GLF_dead0j = 0;
  int donor_replacementGLF_dead0stack[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int donor_replacementGLF_dead0top = 0;
  int x_54 = 0;
  vec4 matrix_b = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  int b = 0;
  k = 0;
  {
    for(; (k < 4); k = (k + 1)) {
      float x_62 = x_6.injectionSwitch.y;
      if ((0.0f > x_62)) {
        GLF_dead0j = 1;
        while (true) {
          int x_13 = donor_replacementGLF_dead0stack[0];
          if ((1 <= x_13)) {
          } else {
            break;
          }
        }
        if (((donor_replacementGLF_dead0top >= 0) & (donor_replacementGLF_dead0top < 9))) {
          int x_17 = (donor_replacementGLF_dead0top + 1);
          donor_replacementGLF_dead0top = x_17;
          x_54 = x_17;
        } else {
          x_54 = 0;
        }
        donor_replacementGLF_dead0stack[x_54] = 1;
      }
      matrix_b = vec4(0.0f, 0.0f, 0.0f, 0.0f);
      b = 3;
      {
        for(; (b >= 0); b = (b - 1)) {
          int x_20 = b;
          float x_87 = matrix_b[b];
          matrix_b[x_20] = (x_87 - 1.0f);
        }
      }
    }
  }
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
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
ERROR: 0:34: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:34: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



