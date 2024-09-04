SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec2 a = vec2(0.0f);
  vec2 b = vec2(0.0f);
  a = vec2(1.0f);
  float x_25 = a.x;
  a[0u] = (x_25 + 0.5f);
  vec2 x_28 = a;
  b = fract(x_28);
  float x_31 = b.x;
  if ((x_31 == 0.5f)) {
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
