SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[4];
};

struct strided_arr_1 {
  int el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf1 x_6;
uniform buf0 x_9;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec2 v1 = vec2(0.0f);
  ivec2 v2 = ivec2(0);
  vec2 v3 = vec2(0.0f);
  bool x_66 = false;
  bool x_67_phi = false;
  float x_41 = x_6.x_GLF_uniform_float_values[2].el;
  float x_43 = x_6.x_GLF_uniform_float_values[3].el;
  v1 = sinh(vec2(x_41, x_43));
  int x_47 = x_9.x_GLF_uniform_int_values[0].el;
  v2 = ivec2(x_47, -3000);
  vec2 x_49 = v1;
  ivec2 x_50 = v2;
  v3 = ldexp(x_49, x_50);
  float x_53 = v3.y;
  x_GLF_color = vec4(x_53, x_53, x_53, x_53);
  float x_56 = v3.x;
  float x_58 = x_6.x_GLF_uniform_float_values[0].el;
  bool x_59 = (x_56 > x_58);
  x_67_phi = x_59;
  if (x_59) {
    float x_63 = v3.x;
    float x_65 = x_6.x_GLF_uniform_float_values[1].el;
    x_66 = (x_63 < x_65);
    x_67_phi = x_66;
  }
  bool x_67 = x_67_phi;
  if (x_67) {
    int x_72 = x_9.x_GLF_uniform_int_values[0].el;
    int x_75 = x_9.x_GLF_uniform_int_values[1].el;
    int x_78 = x_9.x_GLF_uniform_int_values[1].el;
    int x_81 = x_9.x_GLF_uniform_int_values[0].el;
    float v = float(x_72);
    float v_1 = float(x_75);
    float v_2 = float(x_78);
    x_GLF_color = vec4(v, v_1, v_2, float(x_81));
  } else {
    int x_85 = x_9.x_GLF_uniform_int_values[1].el;
    float x_86 = float(x_85);
    x_GLF_color = vec4(x_86, x_86, x_86, x_86);
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
