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

struct buf2 {
  strided_arr_1 x_GLF_uniform_int_values[4];
};

struct buf3 {
  int three;
};

struct strided_arr_2 {
  uint el;
};

struct buf0 {
  strided_arr_2 x_GLF_uniform_uint_values[1];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 tint_symbol = vec4(0.0f);
uniform buf1 x_8;
vec4 x_GLF_color = vec4(0.0f);
uniform buf2 x_12;
uniform buf3 x_14;
uniform buf0 x_16;
void func0_() {
  vec4 tmp = vec4(0.0f);
  if ((tint_symbol.x > x_8.x_GLF_uniform_float_values[1].el)) {
    tmp = x_GLF_color;
  }
  x_GLF_color = tmp;
}
int func1_() {
  int a = 0;
  a = x_12.x_GLF_uniform_int_values[1].el;
  {
    while(true) {
      if ((a < x_12.x_GLF_uniform_int_values[3].el)) {
      } else {
        break;
      }
      if ((x_14.three > x_12.x_GLF_uniform_int_values[1].el)) {
        func0_();
        a = x_12.x_GLF_uniform_int_values[3].el;
      } else {
        func0_();
      }
      {
      }
      continue;
    }
  }
  int x_144 = a;
  return x_144;
}
void main_1() {
  int a_1 = 0;
  int i = 0;
  int j = 0;
  if ((tint_symbol.x > x_8.x_GLF_uniform_float_values[1].el)) {
    x_GLF_color = vec4(x_8.x_GLF_uniform_float_values[0].el, x_8.x_GLF_uniform_float_values[1].el, x_8.x_GLF_uniform_float_values[0].el, x_8.x_GLF_uniform_float_values[2].el);
  } else {
    x_GLF_color = unpackSnorm4x8(x_16.x_GLF_uniform_uint_values[0].el);
  }
  a_1 = x_12.x_GLF_uniform_int_values[2].el;
  i = 0;
  {
    while(true) {
      if ((i < 5)) {
      } else {
        break;
      }
      j = 0;
      {
        while(true) {
          if ((j < 2)) {
          } else {
            break;
          }
          int x_91 = func1_();
          a_1 = (a_1 + x_91);
          {
            j = (j + 1);
          }
          continue;
        }
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  if ((a_1 == x_12.x_GLF_uniform_int_values[0].el)) {
    x_GLF_color[2u] = (x_GLF_color.z - x_8.x_GLF_uniform_float_values[0].el);
  }
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
