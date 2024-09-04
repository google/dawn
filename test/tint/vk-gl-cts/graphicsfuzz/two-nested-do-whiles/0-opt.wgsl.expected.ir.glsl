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


vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_7;
vec4 tint_symbol = vec4(0.0f);
void main_1() {
  int i = 0;
  int i_1 = 0;
  int i_2 = 0;
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  i = 0;
  float x_35 = x_7.injectionSwitch.y;
  if ((x_35 < 0.0f)) {
  } else {
    bool x_42 = false;
    float x_41 = tint_symbol.y;
    x_42 = (x_41 < -1.0f);
    if (x_42) {
    } else {
      {
        while(true) {
          int x_50 = i;
          if ((x_50 >= 256)) {
            break;
          }
          {
            while(true) {
              i_1 = 0;
              {
                while(true) {
                  int x_58 = i_1;
                  if ((x_58 < 1)) {
                  } else {
                    break;
                  }
                  if (x_42) {
                    i_2 = 0;
                    {
                      while(true) {
                        int x_66 = i_2;
                        if ((x_66 < 1)) {
                        } else {
                          break;
                        }
                        {
                          int x_70 = i_2;
                          i_2 = (x_70 + 1);
                        }
                        continue;
                      }
                    }
                    {
                      int x_72 = i_1;
                      i_1 = (x_72 + 1);
                    }
                    continue;
                  }
                  return;
                }
              }
              {
                if (true) { break; }
              }
              continue;
            }
          }
          {
            if (true) { break; }
          }
          continue;
        }
      }
    }
  }
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
