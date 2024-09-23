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
      int v_1 = i;
      if ((v_1 >= tint_f32_to_i32(v.tint_symbol_1.injectionSwitch.x))) {
        break;
      }
      if ((0.0f > v.tint_symbol_1.injectionSwitch.y)) {
        GLF_dead5cols = 2;
        {
          while(true) {
            if ((GLF_dead5cols <= 4)) {
            } else {
              break;
            }
            GLF_dead5rows = 2;
            {
              while(true) {
                if ((GLF_dead5rows <= 4)) {
                } else {
                  break;
                }
                GLF_dead5c = 0;
                {
                  while(true) {
                    if ((GLF_dead5c < GLF_dead5cols)) {
                    } else {
                      break;
                    }
                    GLF_dead5r = 0;
                    {
                      while(true) {
                        if ((GLF_dead5r < GLF_dead5rows)) {
                        } else {
                          break;
                        }
                        int x_87 = msb10;
                        switch(x_87) {
                          case 1:
                          case 8:
                          {
                            int x_96 = ((((msb10 >= 0) & (msb10 < 9))) ? (msb10) : (0));
                            donor_replacementGLF_dead5sums[x_96] = (donor_replacementGLF_dead5sums[x_96] + 1.0f);
                            break;
                          }
                          default:
                          {
                            break;
                          }
                        }
                        {
                          GLF_dead5r = (GLF_dead5r + 1);
                        }
                        continue;
                      }
                    }
                    {
                      GLF_dead5c = (GLF_dead5c + 1);
                    }
                    continue;
                  }
                }
                msb10 = (msb10 + 1);
                {
                  GLF_dead5rows = (GLF_dead5rows + 1);
                }
                continue;
              }
            }
            {
              GLF_dead5cols = (GLF_dead5cols + 1);
            }
            continue;
          }
        }
      }
      i = (i + 1);
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
ERROR: 0:72: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:72: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
