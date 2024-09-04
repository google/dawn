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
      int x_30_phi = 0;
      bool x_48_phi = false;
      x_30_phi = 0;
      {
        while(true) {
          int x_31 = 0;
          int x_30 = x_30_phi;
          x_48_phi = false;
          if ((x_30 < 10)) {
          } else {
            break;
          }
          float x_37 = tint_symbol.y;
          if ((x_37 < 0.0f)) {
            float x_42 = tint_symbol.x;
            if ((x_42 < 0.0f)) {
              x_48_phi = false;
              break;
            } else {
              {
                x_31 = (x_30 + 1);
                x_30_phi = x_31;
              }
              continue;
            }
          }
          x_47();
          {
            x_31 = (x_30 + 1);
            x_30_phi = x_31;
          }
          continue;
        }
      }
      bool x_48 = x_48_phi;
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
