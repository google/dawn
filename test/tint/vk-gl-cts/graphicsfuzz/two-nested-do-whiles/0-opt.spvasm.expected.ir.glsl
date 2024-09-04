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
  if ((x_7.injectionSwitch.y < 0.0f)) {
  } else {
    bool x_42 = false;
    x_42 = (tint_symbol.y < -1.0f);
    if (x_42) {
    } else {
      {
        while(true) {
          if ((i >= 256)) {
            break;
          }
          {
            while(true) {
              i_1 = 0;
              {
                while(true) {
                  if ((i_1 < 1)) {
                  } else {
                    break;
                  }
                  if (x_42) {
                    i_2 = 0;
                    {
                      while(true) {
                        if ((i_2 < 1)) {
                        } else {
                          break;
                        }
                        {
                          i_2 = (i_2 + 1);
                        }
                        continue;
                      }
                    }
                    {
                      i_1 = (i_1 + 1);
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
