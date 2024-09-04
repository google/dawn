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
  vec3 x_42 = vec3(0.0f);
  vec3 x_43 = vec3(0.0f);
  bool x_56 = false;
  x_40 = false;
  x_42 = vec3(0.0f);
  {
    while(true) {
      float x_47 = x_5.injectionSwitch.y;
      x_43 = x_42;
      if ((x_47 < 0.0f)) {
        color = vec3(1.0f);
        x_43 = vec3(1.0f);
      }
      {
        x_40 = x_40;
        x_42 = x_43;
        if (true) { break; }
      }
      continue;
    }
  }
  x_36 = false;
  x_56 = x_40;
  {
    while(true) {
      bool x_62 = false;
      x_7 = 0;
      x_62 = x_56;
      {
        while(true) {
          bool x_68 = true;
          if (true) {
          } else {
            break;
          }
          x_36 = true;
          x_37 = true;
          break;
        }
      }
      if (true) {
        break;
      }
      x_36 = true;
      break;
    }
  }
  x_38 = true;
  x_GLF_color = (vec4(x_43.x, x_43.y, x_43.z, 1.0f) + vec4(1.0f));
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
