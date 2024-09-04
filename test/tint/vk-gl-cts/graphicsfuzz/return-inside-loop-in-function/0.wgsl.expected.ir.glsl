SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  bool x_36 = false;
  vec3 x_37 = vec3(0.0f);
  int x_6 = 0;
  vec3 x_38 = vec3(0.0f);
  vec3 x_51 = vec3(0.0f);
  vec3 x_54 = vec3(0.0f);
  bool x_40_phi = false;
  vec3 x_55_phi = vec3(0.0f);
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  x_36 = false;
  x_40_phi = false;
  {
    while(true) {
      bool x_45 = false;
      bool x_45_phi = false;
      int x_7_phi = 0;
      vec3 x_51_phi = vec3(0.0f);
      bool x_52_phi = false;
      bool x_40 = x_40_phi;
      x_6 = 0;
      x_45_phi = x_40;
      x_7_phi = 0;
      {
        while(true) {
          x_45 = x_45_phi;
          int x_7 = x_7_phi;
          x_51_phi = vec3(0.0f);
          x_52_phi = x_45;
          if ((x_7 < 0)) {
          } else {
            break;
          }
          x_36 = true;
          x_37 = vec3(1.0f);
          x_51_phi = vec3(1.0f);
          x_52_phi = true;
          break;
        }
      }
      x_51 = x_51_phi;
      bool x_52 = x_52_phi;
      x_55_phi = x_51;
      if (x_52) {
        break;
      }
      x_54 = vec3(0.0f);
      x_36 = true;
      x_55_phi = x_54;
      break;
    }
  }
  vec3 x_55 = x_55_phi;
  x_38 = x_55;
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
vec3 GLF_live4drawShape_() {
  bool x_57 = false;
  vec3 x_58 = vec3(0.0f);
  int i = 0;
  vec3 x_71 = vec3(0.0f);
  vec3 x_74 = vec3(0.0f);
  bool x_60_phi = false;
  vec3 x_75_phi = vec3(0.0f);
  x_60_phi = false;
  {
    while(true) {
      bool x_65 = false;
      bool x_65_phi = false;
      int x_8_phi = 0;
      vec3 x_71_phi = vec3(0.0f);
      bool x_72_phi = false;
      bool x_60 = x_60_phi;
      i = 0;
      x_65_phi = x_60;
      x_8_phi = 0;
      {
        while(true) {
          x_65 = x_65_phi;
          int x_8 = x_8_phi;
          x_71_phi = vec3(0.0f);
          x_72_phi = x_65;
          if ((x_8 < 0)) {
          } else {
            break;
          }
          x_57 = true;
          x_58 = vec3(1.0f);
          x_71_phi = vec3(1.0f);
          x_72_phi = true;
          break;
        }
      }
      x_71 = x_71_phi;
      bool x_72 = x_72_phi;
      x_75_phi = x_71;
      if (x_72) {
        break;
      }
      x_74 = vec3(0.0f);
      x_57 = true;
      x_75_phi = x_74;
      break;
    }
  }
  vec3 x_75 = x_75_phi;
  return x_75;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
