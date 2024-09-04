SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[3];
};

struct strided_arr_1 {
  int el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_int_values[3];
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
  float A1[3] = float[3](0.0f, 0.0f, 0.0f);
  int a = 0;
  float b = 0.0f;
  bool c = false;
  bool x_36 = false;
  float x_38 = x_6.x_GLF_uniform_float_values[2].el;
  float x_40 = x_6.x_GLF_uniform_float_values[0].el;
  float x_42 = x_6.x_GLF_uniform_float_values[1].el;
  A1 = float[3](x_38, x_40, x_42);
  int x_45 = x_9.x_GLF_uniform_int_values[0].el;
  int x_47 = x_9.x_GLF_uniform_int_values[0].el;
  int x_49 = x_9.x_GLF_uniform_int_values[1].el;
  a = min(max(x_45, x_47), x_49);
  int x_51 = a;
  int x_53 = x_9.x_GLF_uniform_int_values[1].el;
  int x_55 = x_9.x_GLF_uniform_int_values[0].el;
  float x_58 = A1[min(max(x_51, x_53), x_55)];
  b = x_58;
  float x_59 = b;
  int x_61 = x_9.x_GLF_uniform_int_values[1].el;
  float x_63 = A1[x_61];
  if ((x_59 < x_63)) {
    float x_69 = x_6.x_GLF_uniform_float_values[0].el;
    float x_71 = x_6.x_GLF_uniform_float_values[2].el;
    x_36 = (x_69 > x_71);
  } else {
    float x_74 = x_6.x_GLF_uniform_float_values[0].el;
    int x_76 = x_9.x_GLF_uniform_int_values[2].el;
    float x_78 = A1[x_76];
    x_36 = (x_74 < x_78);
  }
  bool x_80 = x_36;
  c = x_80;
  bool x_81 = c;
  if (x_81) {
    int x_86 = x_9.x_GLF_uniform_int_values[0].el;
    int x_89 = x_9.x_GLF_uniform_int_values[1].el;
    int x_92 = x_9.x_GLF_uniform_int_values[1].el;
    int x_95 = x_9.x_GLF_uniform_int_values[0].el;
    float v = float(x_86);
    float v_1 = float(x_89);
    float v_2 = float(x_92);
    x_GLF_color = vec4(v, v_1, v_2, float(x_95));
  } else {
    float x_99 = x_6.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4(x_99, x_99, x_99, x_99);
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
