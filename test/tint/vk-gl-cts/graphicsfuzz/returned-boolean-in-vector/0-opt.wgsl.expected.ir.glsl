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
void main_1() {
  bool x_36 = false;
  bool x_37 = false;
  int x_7 = 0;
  bool x_38 = false;
  vec3 color = vec3(0.0f);
  bool x_40 = false;
  vec3 x_43 = vec3(0.0f);
  bool x_40_phi = false;
  vec3 x_42_phi = vec3(0.0f);
  bool x_56_phi = false;
  bool x_58_phi = false;
  x_40_phi = false;
  x_42_phi = vec3(0.0f);
  {
    while(true) {
      vec3 x_43_phi = vec3(0.0f);
      x_40 = x_40_phi;
      vec3 x_42 = x_42_phi;
      float x_47 = x_5.injectionSwitch.y;
      x_43_phi = x_42;
      if ((x_47 < 0.0f)) {
        color = vec3(1.0f);
        x_43_phi = vec3(1.0f);
      }
      x_43 = x_43_phi;
      {
        x_40_phi = x_40;
        x_42_phi = x_43;
        if (true) { break; }
      }
      continue;
    }
  }
  x_36 = false;
  x_56_phi = x_40;
  x_58_phi = false;
  {
    while(true) {
      bool x_62 = false;
      bool x_62_phi = false;
      bool x_64_phi = false;
      int x_65_phi = 0;
      bool x_70_phi = false;
      bool x_71_phi = false;
      bool x_56 = x_56_phi;
      bool x_58 = x_58_phi;
      x_7 = 0;
      x_62_phi = x_56;
      x_64_phi = false;
      x_65_phi = 0;
      {
        while(true) {
          x_62 = x_62_phi;
          bool x_64 = x_64_phi;
          int x_65 = x_65_phi;
          bool x_68 = true;
          x_70_phi = x_62;
          x_71_phi = false;
          if (true) {
          } else {
            break;
          }
          x_36 = true;
          x_37 = true;
          x_70_phi = true;
          x_71_phi = true;
          break;
        }
      }
      bool x_70 = x_70_phi;
      bool x_71 = x_71_phi;
      if (true) {
        break;
      }
      x_36 = true;
      break;
    }
  }
  x_38 = true;
  float x_73 = 1.0f;
  vec4 v = vec4(x_43.x, x_43.y, x_43.z, 1.0f);
  x_GLF_color = (v + vec4(x_73, x_73, x_73, x_73));
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
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
