SKIP: FAILED

#version 310 es

struct buf0 {
  float one;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_5;
void main_1() {
  bool x_30 = false;
  bool x_31 = false;
  x_GLF_color = vec4(0.0f);
  bool x_24 = (x_5.one < 0.0f);
  x_31 = x_24;
  if (!(x_24)) {
    x_30 = (x_5.one < 1.0f);
    x_31 = x_30;
  }
  if (x_31) {
    return;
  }
  if ((x_5.one < 0.0f)) {
    {
      while(true) {
        if ((x_5.one < 0.0f)) {
        } else {
          break;
        }
        x_GLF_color = vec4(1.0f);
        break;
      }
    }
  } else {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
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
