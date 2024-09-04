SKIP: FAILED

#version 310 es

struct buf0 {
  int zero;
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
  a = vec2(1.0f);
  int x_38 = x_6.zero;
  if ((x_38 == 1)) {
    float x_43 = a.x;
    a[0u] = (x_43 + 1.0f);
  }
  float x_47 = a.y;
  b = (vec2(x_47, x_47) + vec2(2.0f, 3.0f));
  vec2 x_50 = b;
  if (all((x_50 == vec2(3.0f, 4.0f)))) {
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
ERROR: 0:8: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
