SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[3];
};

struct strided_arr_1 {
  int el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_int_values[3];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
uniform buf1 x_10;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float f0 = 0.0f;
  float f1 = 0.0f;
  int i = 0;
  bool x_63 = false;
  bool x_64 = false;
  f0 = x_6.x_GLF_uniform_float_values[0].el;
  f1 = x_6.x_GLF_uniform_float_values[0].el;
  i = x_10.x_GLF_uniform_int_values[1].el;
  {
    while(true) {
      if ((i < x_10.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      f0 = abs((1.10000002384185791016f * f0));
      f1 = f0;
      {
        i = (i + 1);
      }
      continue;
    }
  }
  bool x_57 = (f1 > x_6.x_GLF_uniform_float_values[1].el);
  x_64 = x_57;
  if (x_57) {
    x_63 = (f1 < x_6.x_GLF_uniform_float_values[2].el);
    x_64 = x_63;
  }
  if (x_64) {
    float v = float(x_10.x_GLF_uniform_int_values[2].el);
    float v_1 = float(x_10.x_GLF_uniform_int_values[1].el);
    float v_2 = float(x_10.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v, v_1, v_2, float(x_10.x_GLF_uniform_int_values[2].el));
  } else {
    x_GLF_color = vec4(float(x_10.x_GLF_uniform_int_values[1].el));
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
