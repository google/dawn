SKIP: FAILED

#version 310 es

struct buf0 {
  vec2 twoandthree;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec2 a = vec2(0.0f);
  vec2 b = vec2(0.0f);
  bool x_46 = false;
  bool x_47 = false;
  a = x_6.twoandthree;
  float v = a.x;
  b = vec2(v, clamp(a, vec2(1.0f), vec2(1.0f))[1u]);
  bool x_41 = (b.x == 2.0f);
  x_47 = x_41;
  if (x_41) {
    x_46 = (b.y == 1.0f);
    x_47 = x_46;
  }
  if (x_47) {
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
