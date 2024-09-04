SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 tint_symbol = vec4(0.0f);
vec4 x_GLF_color = vec4(0.0f);
bool continue_execution = true;
void x_47() {
  continue_execution = false;
}
void main_1() {
  {
    while(true) {
      int x_30 = 0;
      bool x_48 = false;
      x_30 = 0;
      {
        while(true) {
          int x_31 = 0;
          x_48 = false;
          if ((x_30 < 10)) {
          } else {
            break;
          }
          if ((tint_symbol.y < 0.0f)) {
            if ((tint_symbol.x < 0.0f)) {
              x_48 = false;
              break;
            } else {
              {
                x_31 = (x_30 + 1);
                x_30 = x_31;
              }
              continue;
            }
          }
          x_47();
          x_48 = true;
          break;
        }
      }
      if (x_48) {
        break;
      }
      x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
      break;
    }
  }
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
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
