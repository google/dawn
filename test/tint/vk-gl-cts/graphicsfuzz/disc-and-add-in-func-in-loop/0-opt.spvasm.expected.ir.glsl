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


uniform buf0 x_5;
vec4 x_GLF_color = vec4(0.0f);
bool continue_execution = true;
void x_51() {
  continue_execution = false;
}
void main_1() {
  {
    while(true) {
      bool x_30 = false;
      bool x_31 = false;
      x_30 = false;
      {
        while(true) {
          {
            while(true) {
              vec4 x_52 = vec4(0.0f);
              vec4 x_54 = vec4(0.0f);
              vec4 x_55 = vec4(0.0f);
              float x_36 = x_5.injectionSwitch.y;
              x_31 = x_30;
              if ((x_36 > 0.0f)) {
              } else {
                break;
              }
              {
                while(true) {
                  float x_46 = x_5.injectionSwitch.x;
                  if ((x_46 > 0.0f)) {
                    x_51();
                    x_52 = vec4(0.0f);
                    x_55 = x_52;
                    break;
                  }
                  x_54 = (vec4(1.0f, 0.0f, 0.0f, 1.0f) + vec4(x_46));
                  x_55 = x_54;
                  break;
                }
              }
              x_GLF_color = x_55;
              x_31 = true;
              break;
            }
          }
          if (x_31) {
            break;
          } else {
            {
              x_30 = x_31;
            }
            continue;
          }
          /* unreachable */
        }
      }
      if (x_31) {
        break;
      }
      break;
    }
  }
}
main_out main() {
  main_1();
  main_out v = main_out(x_GLF_color);
  if (!(continue_execution)) {
    discard;
  }
  return v;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
