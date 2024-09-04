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
  bool x_31_phi = false;
  x_GLF_color = vec4(0.0f);
  float x_23 = x_5.one;
  bool x_24 = (x_23 < 0.0f);
  x_31_phi = x_24;
  if (!(x_24)) {
    float x_29 = x_5.one;
    x_30 = (x_29 < 1.0f);
    x_31_phi = x_30;
  }
  bool x_31 = x_31_phi;
  if (x_31) {
    return;
  }
  float x_35 = x_5.one;
  if ((x_35 < 0.0f)) {
    {
      while(true) {
        float x_45 = x_5.one;
        if ((x_45 < 0.0f)) {
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
