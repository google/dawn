SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[2];
};

struct strided_arr_1 {
  int el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
uniform buf1 x_9;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float arr[5] = float[5](0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int i = 0;
  int j = 0;
  arr = float[5](x_6.x_GLF_uniform_float_values[0].el, x_6.x_GLF_uniform_float_values[0].el, x_6.x_GLF_uniform_float_values[0].el, x_6.x_GLF_uniform_float_values[0].el, x_6.x_GLF_uniform_float_values[0].el);
  i = x_9.x_GLF_uniform_int_values[1].el;
  j = 0;
  {
    while(true) {
      if ((i < x_9.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      if ((j < -1)) {
        break;
      }
      int x_63 = j;
      arr[x_63] = (arr[j] + 1.0f);
      {
        i = (i + 1);
        j = (j + 1);
      }
      continue;
    }
  }
  x_GLF_color = vec4(x_6.x_GLF_uniform_float_values[0].el, x_6.x_GLF_uniform_float_values[1].el, x_6.x_GLF_uniform_float_values[1].el, x_6.x_GLF_uniform_float_values[0].el);
  i = x_9.x_GLF_uniform_int_values[1].el;
  {
    while(true) {
      if ((i < x_9.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      if (!((arr[i] == 2.0f))) {
        x_GLF_color = vec4(x_6.x_GLF_uniform_float_values[1].el);
      }
      {
        i = (i + 1);
      }
      continue;
    }
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
