SKIP: FAILED

#version 310 es

struct buf0 {
  float fourtytwo;
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
  bool x_37_phi = false;
  float x_23 = x_5.fourtytwo;
  float x_25 = x_5.fourtytwo;
  bool x_27 = (clamp(1.0f, x_23, x_25) > 42.0f);
  x_37_phi = x_27;
  if (!(x_27)) {
    float x_32 = x_5.fourtytwo;
    float x_34 = x_5.fourtytwo;
    x_36 = (clamp(1.0f, x_32, x_34) < 42.0f);
    x_37_phi = x_36;
  }
  bool x_37 = x_37_phi;
  if (x_37) {
    x_GLF_color = vec4(0.0f);
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
