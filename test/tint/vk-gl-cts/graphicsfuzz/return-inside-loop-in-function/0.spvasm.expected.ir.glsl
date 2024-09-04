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
  bool x_40 = false;
  vec3 x_51 = vec3(0.0f);
  vec3 x_54 = vec3(0.0f);
  vec3 x_55 = vec3(0.0f);
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  x_36 = false;
  x_40 = false;
  {
    while(true) {
      bool x_45 = false;
      int x_7 = 0;
      bool x_52 = false;
      x_6 = 0;
      x_45 = x_40;
      x_7 = 0;
      {
        while(true) {
          x_51 = vec3(0.0f);
          x_52 = x_45;
          if ((x_7 < 0)) {
          } else {
            break;
          }
          x_36 = true;
          x_37 = vec3(1.0f);
          x_51 = vec3(1.0f);
          x_52 = true;
          break;
        }
      }
      x_55 = x_51;
      if (x_52) {
        break;
      }
      x_54 = vec3(0.0f);
      x_36 = true;
      x_55 = x_54;
      break;
    }
  }
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
  bool x_60 = false;
  vec3 x_71 = vec3(0.0f);
  vec3 x_74 = vec3(0.0f);
  vec3 x_75 = vec3(0.0f);
  x_60 = false;
  {
    while(true) {
      bool x_65 = false;
      int x_8 = 0;
      bool x_72 = false;
      i = 0;
      x_65 = x_60;
      x_8 = 0;
      {
        while(true) {
          x_71 = vec3(0.0f);
          x_72 = x_65;
          if ((x_8 < 0)) {
          } else {
            break;
          }
          x_57 = true;
          x_58 = vec3(1.0f);
          x_71 = vec3(1.0f);
          x_72 = true;
          break;
        }
      }
      x_75 = x_71;
      if (x_72) {
        break;
      }
      x_74 = vec3(0.0f);
      x_57 = true;
      x_75 = x_74;
      break;
    }
  }
  return x_75;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
