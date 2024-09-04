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
void main_1() {
  bool x_30 = false;
  float x_47 = 0.0f;
  float x_29 = x_6.injectionSwitch.x;
  x_30 = (x_29 > 1.0f);
  if (x_30) {
    {
      while(true) {
        float x_47_phi = 0.0f;
        {
          while(true) {
            float x_41 = tint_symbol.x;
            if ((x_41 < 0.0f)) {
              if (x_30) {
                x_47_phi = 1.0f;
                break;
              } else {
                {
                }
                continue;
              }
            }
            x_47_phi = 0.0f;
            break;
          }
        }
        x_47 = x_47_phi;
        break;
      }
    }
    vec4 x_48_1 = vec4(0.0f);
    x_48_1[1u] = x_47;
    vec4 x_48 = x_48_1;
    x_GLF_color = x_48;
  }
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
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
