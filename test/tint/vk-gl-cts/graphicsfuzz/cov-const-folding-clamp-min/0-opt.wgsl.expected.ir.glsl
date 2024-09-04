SKIP: FAILED

#version 310 es

struct buf0 {
  float zero;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_5;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float x_25 = x_5.zero;
  float x_28 = x_5.zero;
  float v = clamp(2.0f, x_25, 1.0f);
  if (any((vec4(v, clamp(-1.0f, 0.0f, x_28), 0.0f, 1.0f) != vec4(1.0f, 0.0f, 0.0f, 1.0f)))) {
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
