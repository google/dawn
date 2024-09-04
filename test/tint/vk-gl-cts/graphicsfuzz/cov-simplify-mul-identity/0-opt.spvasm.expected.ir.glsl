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


uniform buf0 x_7;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec4 v = vec4(0.0f);
  vec4 res = vec4(0.0f);
  v = vec4(8.3999996185302734375f, -864.66497802734375f, 945.41998291015625f, 1.0f);
  float x_31 = x_7.one;
  vec4 v_1 = vec4(x_31, 0.0f, 0.0f, 0.0f);
  vec4 v_2 = vec4(0.0f, x_31, 0.0f, 0.0f);
  vec4 v_3 = vec4(0.0f, 0.0f, x_31, 0.0f);
  mat4 v_4 = mat4(v_1, v_2, v_3, vec4(0.0f, 0.0f, 0.0f, x_31));
  res = (v_4 * v);
  if ((distance(v, res) < 0.00999999977648258209f)) {
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
