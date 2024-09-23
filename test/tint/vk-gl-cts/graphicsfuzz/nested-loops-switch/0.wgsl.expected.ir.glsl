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
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : ((-2147483647 - 1)))) : (2147483647));
}
void main_1() {
  int i = 0;
  int GLF_dead5cols = 0;
  int GLF_dead5rows = 0;
  int GLF_dead5c = 0;
  int GLF_dead5r = 0;
  int msb10 = 0;
  float donor_replacementGLF_dead5sums[9] = float[9](0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  i = 0;
  {
    while(true) {
      int x_45 = i;
      float x_47 = v.tint_symbol_1.injectionSwitch.x;
      if ((x_45 >= tint_f32_to_i32(x_47))) {
        break;
      }
      float x_53 = v.tint_symbol_1.injectionSwitch.y;
      if ((0.0f > x_53)) {
        GLF_dead5cols = 2;
        {
          while(true) {
            int x_61 = GLF_dead5cols;
            if ((x_61 <= 4)) {
            } else {
              break;
            }
            GLF_dead5rows = 2;
            {
              while(true) {
                int x_68 = GLF_dead5rows;
                if ((x_68 <= 4)) {
                } else {
                  break;
                }
                GLF_dead5c = 0;
                {
                  while(true) {
                    int x_75 = GLF_dead5c;
                    int x_76 = GLF_dead5cols;
                    if ((x_75 < x_76)) {
                    } else {
                      break;
                    }
                    GLF_dead5r = 0;
                    {
                      while(true) {
                        int x_83 = GLF_dead5r;
                        int x_84 = GLF_dead5rows;
                        if ((x_83 < x_84)) {
                        } else {
                          break;
                        }
                        int x_87 = msb10;
                        switch(x_87) {
                          case 1:
                          case 8:
                          {
                            int x_90 = msb10;
                            int x_92 = msb10;
                            int x_95 = msb10;
                            int x_96 = ((((x_90 >= 0) & (x_92 < 9))) ? (x_95) : (0));
                            float x_98 = donor_replacementGLF_dead5sums[x_96];
                            donor_replacementGLF_dead5sums[x_96] = (x_98 + 1.0f);
                            break;
                          }
                          default:
                          {
                            break;
                          }
                        }
                        {
                          int x_101 = GLF_dead5r;
                          GLF_dead5r = (x_101 + 1);
                        }
                        continue;
                      }
                    }
                    {
                      int x_103 = GLF_dead5c;
                      GLF_dead5c = (x_103 + 1);
                    }
                    continue;
                  }
                }
                int x_105 = msb10;
                msb10 = (x_105 + 1);
                {
                  int x_107 = GLF_dead5rows;
                  GLF_dead5rows = (x_107 + 1);
                }
                continue;
              }
            }
            {
              int x_109 = GLF_dead5cols;
              GLF_dead5cols = (x_109 + 1);
            }
            continue;
          }
        }
      }
      int x_111 = i;
      i = (x_111 + 1);
      {
        int x_113 = i;
        if (!((x_113 < 200))) { break; }
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
ERROR: 0:83: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:83: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
