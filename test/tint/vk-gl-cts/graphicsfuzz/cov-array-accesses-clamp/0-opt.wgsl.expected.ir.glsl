SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[3];
};

struct strided_arr_1 {
  float el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_float_values[1];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_7;
vec4 tint_symbol = vec4(0.0f);
uniform buf1 x_11;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int arr[3] = int[3](0, 0, 0);
  int a = 0;
  int b = 0;
  int c = 0;
  int x_40 = x_7.x_GLF_uniform_int_values[1].el;
  int x_42 = x_7.x_GLF_uniform_int_values[1].el;
  int x_44 = x_7.x_GLF_uniform_int_values[1].el;
  arr = int[3](x_40, x_42, x_44);
  int x_47 = x_7.x_GLF_uniform_int_values[0].el;
  int x_49 = arr[x_47];
  a = x_49;
  int x_50 = a;
  b = (x_50 - 1);
  float x_53 = tint_symbol.x;
  float x_55 = x_11.x_GLF_uniform_float_values[0].el;
  if ((x_53 < x_55)) {
    int x_59 = b;
    b = (x_59 + 1);
  }
  int x_62 = x_7.x_GLF_uniform_int_values[0].el;
  c = x_62;
  int x_63 = c;
  int x_65 = x_7.x_GLF_uniform_int_values[1].el;
  int x_67 = x_7.x_GLF_uniform_int_values[2].el;
  int x_69 = b;
  arr[min(max(x_63, x_65), x_67)] = x_69;
  int x_72 = x_7.x_GLF_uniform_int_values[0].el;
  int x_74 = arr[x_72];
  int x_77 = x_7.x_GLF_uniform_int_values[1].el;
  int x_79 = arr[x_77];
  int x_82 = x_7.x_GLF_uniform_int_values[1].el;
  int x_84 = arr[x_82];
  int x_87 = x_7.x_GLF_uniform_int_values[2].el;
  int x_89 = arr[x_87];
  float v = float(x_74);
  float v_1 = float(x_79);
  float v_2 = float(x_84);
  x_GLF_color = vec4(v, v_1, v_2, float(x_89));
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
