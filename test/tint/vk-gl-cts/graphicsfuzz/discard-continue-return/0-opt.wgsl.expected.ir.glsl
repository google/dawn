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


vec4 tint_symbol = vec4(0.0f);
uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
bool continue_execution = true;
void main_1() {
  {
    while(true) {
      bool x_46_phi = false;
      {
        while(true) {
          float x_37 = tint_symbol.x;
          if ((x_37 < 0.0f)) {
            float x_42 = x_6.injectionSwitch.y;
            if ((1.0f > x_42)) {
              continue_execution = false;
            } else {
              {
                x_46_phi = false;
                if (true) { break; }
              }
              continue;
            }
          }
          x_46_phi = true;
          break;
        }
      }
      bool x_46 = x_46_phi;
      if (x_46) {
        break;
      }
      break;
    }
  }
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
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
