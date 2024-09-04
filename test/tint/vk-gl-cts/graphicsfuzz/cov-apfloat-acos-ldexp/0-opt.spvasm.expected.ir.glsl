SKIP: FAILED

#version 310 es

struct buf0 {
  int two;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec2 v = vec2(0.0f);
  float d = 0.0f;
  v = acos(ldexp(vec2(0.10000000149011611938f), ivec2(x_6.two, 3)));
  d = distance(v, vec2(1.15927994251251220703f, 0.64349997043609619141f));
  if ((d < 0.00999999977648258209f)) {
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
