SKIP: FAILED

#version 310 es

struct buf0 {
  vec4 r;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_7;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float f = 0.0f;
  vec4 v = vec4(0.0f);
  f = 1.0f;
  float v_1 = sin(f);
  float v_2 = cos(f);
  float v_3 = exp2(f);
  v = vec4(v_1, v_2, v_3, log(f));
  if ((distance(v, x_7.r) < 0.10000000149011611938f)) {
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
