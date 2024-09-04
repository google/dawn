SKIP: FAILED

#version 310 es

struct buf0 {
  float quarter;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_7;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec4 N = vec4(0.0f);
  vec4 I = vec4(0.0f);
  vec4 Nref = vec4(0.0f);
  vec4 v = vec4(0.0f);
  N = vec4(1.0f, 2.0f, 3.0f, 4.0f);
  float x_44 = x_7.quarter;
  I = vec4(4.0f, 87.589996337890625f, x_44, 92.51000213623046875f);
  Nref = vec4(17.049999237060546875f, -6.09999990463256835938f, 4329.37060546875f, 2.70000004768371582031f);
  vec4 x_46 = N;
  vec4 x_47 = I;
  vec4 x_48 = Nref;
  v = faceforward(x_46, x_47, x_48);
  vec4 x_50 = v;
  if (all((x_50 == vec4(-1.0f, -2.0f, -3.0f, -4.0f)))) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f);
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
