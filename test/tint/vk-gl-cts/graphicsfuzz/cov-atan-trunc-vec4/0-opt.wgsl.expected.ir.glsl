SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf1 {
  strided_arr x_GLF_uniform_int_values[2];
};

struct strided_arr_1 {
  float el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_float_values[2];
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
  vec4 v = vec4(0.0f);
  float f = 0.0f;
  bool x_56 = false;
  bool x_57_phi = false;
  int x_32 = x_6.x_GLF_uniform_int_values[0].el;
  int x_35 = x_6.x_GLF_uniform_int_values[0].el;
  int x_38 = x_6.x_GLF_uniform_int_values[0].el;
  float v_1 = float(x_32);
  float v_2 = float(x_35);
  v = vec4(v_1, v_2, -621.59600830078125f, float(x_38));
  vec4 x_41 = v;
  f = atan(trunc(x_41))[2u];
  float x_45 = f;
  float x_47 = x_9.x_GLF_uniform_float_values[0].el;
  bool x_49 = (x_45 > -(x_47));
  x_57_phi = x_49;
  if (x_49) {
    float x_52 = f;
    float x_54 = x_9.x_GLF_uniform_float_values[1].el;
    x_56 = (x_52 < -(x_54));
    x_57_phi = x_56;
  }
  bool x_57 = x_57_phi;
  if (x_57) {
    int x_62 = x_6.x_GLF_uniform_int_values[1].el;
    int x_65 = x_6.x_GLF_uniform_int_values[0].el;
    int x_68 = x_6.x_GLF_uniform_int_values[0].el;
    int x_71 = x_6.x_GLF_uniform_int_values[1].el;
    float v_3 = float(x_62);
    float v_4 = float(x_65);
    float v_5 = float(x_68);
    x_GLF_color = vec4(v_3, v_4, v_5, float(x_71));
  } else {
    int x_75 = x_6.x_GLF_uniform_int_values[0].el;
    float x_76 = float(x_75);
    x_GLF_color = vec4(x_76, x_76, x_76, x_76);
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
