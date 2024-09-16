SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct buf0 {
  vec2 injectionSwitch;
};

struct main_out {
  vec4 x_GLF_color_1;
};

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  buf0 tint_symbol_1;
} v;
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_loc0_Output;
void main_1() {
  int k = 0;
  int GLF_dead0j = 0;
  int donor_replacementGLF_dead0stack[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int donor_replacementGLF_dead0top = 0;
  int x_54 = 0;
  vec4 matrix_b = vec4(0.0f);
  int b = 0;
  k = 0;
  {
    while(true) {
      if ((k < 4)) {
      } else {
        break;
      }
      if ((0.0f > v.tint_symbol_1.injectionSwitch.y)) {
        GLF_dead0j = 1;
        {
          while(true) {
            if ((1 <= donor_replacementGLF_dead0stack[0])) {
            } else {
              break;
            }
            {
            }
            continue;
          }
        }
        if (((donor_replacementGLF_dead0top >= 0) & (donor_replacementGLF_dead0top < 9))) {
          int x_17 = (donor_replacementGLF_dead0top + 1);
          donor_replacementGLF_dead0top = x_17;
          x_54 = x_17;
        } else {
          x_54 = 0;
        }
        int x_18 = x_54;
        donor_replacementGLF_dead0stack[x_18] = 1;
      }
      matrix_b = vec4(0.0f);
      b = 3;
      {
        while(true) {
          if ((b >= 0)) {
          } else {
            break;
          }
          int x_20 = b;
          matrix_b[x_20] = (matrix_b[b] - 1.0f);
          {
            b = (b - 1);
          }
          continue;
        }
      }
      {
        k = (k + 1);
      }
      continue;
    }
  }
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
main_out tint_symbol_inner() {
  main_1();
  return main_out(x_GLF_color);
}
void main() {
  tint_symbol_loc0_Output = tint_symbol_inner().x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:48: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:48: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
