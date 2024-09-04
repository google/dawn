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
  bool x_36 = (x_7.injectionSwitch.x > x_7.injectionSwitch.y);
  if (x_36) {
    return;
  }
  bool x_41 = (tint_symbol.x < 0.0f);
  {
    while(true) {
      if ((loop_count < 100)) {
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
            if ((loop_count < 100)) {
            } else {
              break;
            }
            {
              loop_count = (loop_count + 1);
            }
            continue;
          }
        }
      }
      {
        loop_count = (loop_count + 1);
      }
      continue;
    }
  }
  if ((loop_count >= 100)) {
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
