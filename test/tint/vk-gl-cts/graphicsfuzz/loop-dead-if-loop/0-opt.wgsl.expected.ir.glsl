SKIP: FAILED

#version 310 es

struct buf0 {
  vec2 injectionSwitch;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
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
      int x_12 = k;
      if ((x_12 < 4)) {
      } else {
        break;
      }
      float x_62 = x_6.injectionSwitch.y;
      if ((0.0f > x_62)) {
        GLF_dead0j = 1;
        {
          while(true) {
            int x_13 = donor_replacementGLF_dead0stack[0];
            if ((1 <= x_13)) {
            } else {
              break;
            }
            {
            }
            continue;
          }
        }
        int x_14 = donor_replacementGLF_dead0top;
        int x_15 = donor_replacementGLF_dead0top;
        if (((x_14 >= 0) & (x_15 < 9))) {
          int x_16 = donor_replacementGLF_dead0top;
          int x_17 = (x_16 + 1);
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
          int x_19 = b;
          if ((x_19 >= 0)) {
          } else {
            break;
          }
          int x_20 = b;
          int x_21 = b;
          float x_87 = matrix_b[x_21];
          matrix_b[x_20] = (x_87 - 1.0f);
          {
            int x_22 = b;
            b = (x_22 - 1);
          }
          continue;
        }
      }
      {
        int x_24 = k;
        k = (x_24 + 1);
      }
      continue;
    }
  }
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
