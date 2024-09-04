SKIP: FAILED

#version 310 es

struct buf0 {
  int zero;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
vec4 returnRed_() {
  bool x_33 = false;
  vec4 x_34 = vec4(0.0f);
  vec4 x_48 = vec4(0.0f);
  bool x_36_phi = false;
  vec4 x_51_phi = vec4(0.0f);
  x_36_phi = false;
  {
    while(true) {
      vec4 x_48_phi = vec4(0.0f);
      bool x_49_phi = false;
      bool x_36 = x_36_phi;
      {
        while(true) {
          int x_44 = x_6.zero;
          if ((x_44 == 1)) {
            x_33 = true;
            x_34 = vec4(1.0f, 0.0f, 0.0f, 1.0f);
            x_48_phi = vec4(1.0f, 0.0f, 0.0f, 1.0f);
            x_49_phi = true;
            break;
          }
          {
            x_48_phi = vec4(0.0f);
            x_49_phi = false;
            if (true) { break; }
          }
          continue;
        }
      }
      x_48 = x_48_phi;
      bool x_49 = x_49_phi;
      x_51_phi = x_48;
      if (x_49) {
        break;
      }
      x_33 = true;
      x_34 = vec4(1.0f, 0.0f, 0.0f, 1.0f);
      x_51_phi = vec4(1.0f, 0.0f, 0.0f, 1.0f);
      break;
    }
  }
  vec4 x_51 = x_51_phi;
  return x_51;
}
void main_1() {
  {
    while(true) {
      vec4 x_30 = returnRed_();
      x_GLF_color = x_30;
      if (false) {
      } else {
        break;
      }
      {
      }
      continue;
    }
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
