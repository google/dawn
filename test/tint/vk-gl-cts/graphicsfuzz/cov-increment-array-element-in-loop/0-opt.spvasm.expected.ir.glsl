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
  float arr[3] = float[3](0.0f, 0.0f, 0.0f);
  int a = 0;
  bool x_69 = false;
  bool x_70 = false;
  bool x_79 = false;
  bool x_80 = false;
  arr = float[3](x_6.x_GLF_uniform_float_values[1].el, x_6.x_GLF_uniform_float_values[0].el, x_6.x_GLF_uniform_float_values[2].el);
  a = 0;
  {
    while(true) {
      if ((a <= x_9.x_GLF_uniform_int_values[1].el)) {
      } else {
        break;
      }
      int x_49 = a;
      a = (a + 1);
      arr[x_49] = x_6.x_GLF_uniform_float_values[0].el;
      {
      }
      continue;
    }
  }
  bool x_60 = (arr[x_9.x_GLF_uniform_int_values[1].el] == x_6.x_GLF_uniform_float_values[0].el);
  x_70 = x_60;
  if (x_60) {
    x_69 = (arr[x_9.x_GLF_uniform_int_values[2].el] == x_6.x_GLF_uniform_float_values[0].el);
    x_70 = x_69;
  }
  x_80 = x_70;
  if (x_70) {
    x_79 = (arr[x_9.x_GLF_uniform_int_values[0].el] == x_6.x_GLF_uniform_float_values[2].el);
    x_80 = x_79;
  }
  if (x_80) {
    x_GLF_color = vec4(arr[x_9.x_GLF_uniform_int_values[1].el], x_6.x_GLF_uniform_float_values[1].el, x_6.x_GLF_uniform_float_values[1].el, x_6.x_GLF_uniform_float_values[0].el);
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
