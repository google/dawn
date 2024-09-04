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
uniform buf0 x_6;
int performPartition_() {
  int GLF_live0i = 0;
  int i = 0;
  int x_11 = 0;
  int x_10_phi = 0;
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  x_10_phi = 0;
  {
    while(true) {
      int x_11_phi = 0;
      int x_10 = x_10_phi;
      bool x_42 = false;
      float x_41 = x_6.injectionSwitch.y;
      x_42 = (x_41 < 0.0f);
      if (x_42) {
        x_11_phi = x_10;
        {
          x_11 = x_11_phi;
          x_10_phi = x_11;
          if (true) { break; }
        }
        continue;
      } else {
        GLF_live0i = 0;
        {
          while(true) {
            bool x_47 = true;
            if (x_42) {
              break;
            }
            return 1;
          }
        }
        if (x_42) {
          {
            while(true) {
              return 1;
            }
          }
        }
        x_11_phi = x_10;
        {
          x_11 = x_11_phi;
          x_10_phi = x_11;
          if (true) { break; }
        }
        continue;
      }
      /* unreachable */
    }
  }
  return x_11;
}
void main_1() {
  int x_9 = performPartition_();
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
