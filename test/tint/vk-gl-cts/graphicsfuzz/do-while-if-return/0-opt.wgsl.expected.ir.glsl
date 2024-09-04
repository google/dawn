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
vec4 x_GLF_color = vec4(0.0f);
int func_() {
  int loop_count = 0;
  int x_38_phi = 0;
  loop_count = 0;
  x_38_phi = 0;
  {
    while(true) {
      int x_39 = 0;
      int x_45_phi = 0;
      int x_38 = x_38_phi;
      int x_43 = (x_38 + 1);
      loop_count = x_43;
      x_45_phi = x_43;
      {
        while(true) {
          int x_45 = x_45_phi;
          x_39 = (x_45 + 1);
          loop_count = x_39;
          float x_50 = x_7.injectionSwitch.x;
          float x_52 = x_7.injectionSwitch.y;
          if ((x_50 < x_52)) {
            return 1;
          }
          float x_57 = x_7.injectionSwitch.x;
          float x_59 = x_7.injectionSwitch.y;
          if ((x_57 < x_59)) {
            break;
          }
          {
            x_45_phi = x_39;
            if (!((x_39 < 100))) { break; }
          }
          continue;
        }
      }
      {
        x_38_phi = x_39;
        if (!((x_39 < 100))) { break; }
      }
      continue;
    }
  }
  return 0;
}
void main_1() {
  int x_31 = func_();
  if ((x_31 == 1)) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(1.0f);
  }
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
