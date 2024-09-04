SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf1 {
  strided_arr x_GLF_uniform_int_values[3];
};

struct strided_arr_1 {
  float el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_float_values[1];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf1 x_7;
vec4 tint_symbol = vec4(0.0f);
uniform buf0 x_11;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int i = 0;
  int arr[2] = int[2](0, 0);
  int a = 0;
  int x_40 = x_7.x_GLF_uniform_int_values[0].el;
  i = x_40;
  {
    while(true) {
      int x_45 = i;
      int x_47 = x_7.x_GLF_uniform_int_values[2].el;
      if ((x_45 < x_47)) {
      } else {
        break;
      }
      int x_50 = i;
      int x_52 = x_7.x_GLF_uniform_int_values[0].el;
      arr[x_50] = x_52;
      {
        int x_54 = i;
        i = (x_54 + 1);
      }
      continue;
    }
  }
  a = -1;
  float x_57 = tint_symbol.y;
  float x_59 = x_11.x_GLF_uniform_float_values[0].el;
  if (!((x_57 < x_59))) {
    int x_64 = a;
    int x_65 = (x_64 + 1);
    a = x_65;
    int x_67 = x_7.x_GLF_uniform_int_values[1].el;
    arr[x_65] = x_67;
  }
  int x_69 = a;
  int x_70 = (x_69 + 1);
  a = x_70;
  int x_72 = x_7.x_GLF_uniform_int_values[2].el;
  arr[x_70] = x_72;
  int x_75 = x_7.x_GLF_uniform_int_values[0].el;
  int x_77 = arr[x_75];
  int x_79 = x_7.x_GLF_uniform_int_values[1].el;
  if ((x_77 == x_79)) {
    int x_84 = a;
    int x_87 = x_7.x_GLF_uniform_int_values[0].el;
    int x_90 = x_7.x_GLF_uniform_int_values[0].el;
    int x_92 = a;
    float v = float(x_84);
    float v_1 = float(x_87);
    float v_2 = float(x_90);
    x_GLF_color = vec4(v, v_1, v_2, float(x_92));
  } else {
    int x_96 = x_7.x_GLF_uniform_int_values[0].el;
    float x_97 = float(x_96);
    x_GLF_color = vec4(x_97, x_97, x_97, x_97);
  }
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
