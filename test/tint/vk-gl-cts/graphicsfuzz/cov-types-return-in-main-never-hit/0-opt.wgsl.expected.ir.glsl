SKIP: FAILED

#version 310 es

struct buf0 {
  int one;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_6;
void main_1() {
  vec4 x_24 = vec4(0.0f);
  x_GLF_color = vec4(0.0f);
  int x_26 = x_6.one;
  if ((x_26 == 0)) {
    return;
  }
  x_24 = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
vec4 func_() {
  return vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
