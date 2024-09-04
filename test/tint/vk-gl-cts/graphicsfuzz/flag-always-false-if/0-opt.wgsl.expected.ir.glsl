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


uniform buf0 x_7;
vec4 tint_symbol = vec4(0.0f);
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int loop_count = 0;
  loop_count = 0;
  float x_33 = x_7.injectionSwitch.x;
  float x_35 = x_7.injectionSwitch.y;
  bool x_36 = (x_33 > x_35);
  if (x_36) {
    return;
  }
  float x_40 = tint_symbol.x;
  bool x_41 = (x_40 < 0.0f);
  {
    while(true) {
      int x_43 = loop_count;
      if ((x_43 < 100)) {
      } else {
        break;
      }
      if (x_36) {
        break;
      }
      if (x_36) {
        x_GLF_color = vec4(1.0f);
      } else {
        if (x_41) {
          return;
        }
      }
      if (x_36) {
        x_GLF_color = vec4(1.0f);
      } else {
        x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
      }
      if (x_36) {
        return;
      }
      if (x_41) {
        {
          while(true) {
            int x_63 = loop_count;
            if ((x_63 < 100)) {
            } else {
              break;
            }
            {
              int x_67 = loop_count;
              loop_count = (x_67 + 1);
            }
            continue;
          }
        }
      }
      {
        int x_69 = loop_count;
        loop_count = (x_69 + 1);
      }
      continue;
    }
  }
  int x_71 = loop_count;
  if ((x_71 >= 100)) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(1.0f);
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
