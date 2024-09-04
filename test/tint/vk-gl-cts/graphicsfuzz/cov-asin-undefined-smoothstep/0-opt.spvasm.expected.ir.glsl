SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[7];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec4 v1 = vec4(0.0f);
  vec4 v2 = vec4(0.0f);
  vec4 v3 = vec4(0.0f);
  vec4 v4 = vec4(0.0f);
  bool x_69 = false;
  bool x_70 = false;
  bool x_77 = false;
  bool x_78 = false;
  bool x_85 = false;
  bool x_86 = false;
  bool x_93 = false;
  bool x_94 = false;
  v1 = vec4(x_6.x_GLF_uniform_float_values[2].el, x_6.x_GLF_uniform_float_values[2].el, x_6.x_GLF_uniform_float_values[0].el, x_6.x_GLF_uniform_float_values[2].el);
  v2 = vec4(1.57079637050628662109f, 1.11976957321166992188f, 1.0f, 0.92729520797729492188f);
  v3 = vec4(x_6.x_GLF_uniform_float_values[0].el);
  v4 = smoothstep(v1, v2, v3);
  x_GLF_color = v4.xywx;
  bool x_62 = (v4.x > x_6.x_GLF_uniform_float_values[4].el);
  x_70 = x_62;
  if (x_62) {
    x_69 = (v4.x < x_6.x_GLF_uniform_float_values[5].el);
    x_70 = x_69;
  }
  x_78 = x_70;
  if (x_70) {
    x_77 = (v4.y > x_6.x_GLF_uniform_float_values[3].el);
    x_78 = x_77;
  }
  x_86 = x_78;
  if (x_78) {
    x_85 = (v4.y < x_6.x_GLF_uniform_float_values[6].el);
    x_86 = x_85;
  }
  x_94 = x_86;
  if (x_86) {
    x_93 = (v4.w == x_6.x_GLF_uniform_float_values[0].el);
    x_94 = x_93;
  }
  if (x_94) {
    x_GLF_color = vec4(x_6.x_GLF_uniform_float_values[0].el, x_6.x_GLF_uniform_float_values[1].el, x_6.x_GLF_uniform_float_values[1].el, x_6.x_GLF_uniform_float_values[0].el);
  } else {
    x_GLF_color = vec4(x_6.x_GLF_uniform_float_values[1].el);
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
