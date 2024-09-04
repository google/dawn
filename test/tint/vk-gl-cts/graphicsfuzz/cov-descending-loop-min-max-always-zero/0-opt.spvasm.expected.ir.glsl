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
  strided_arr_1 x_GLF_uniform_int_values[3];
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
  float f = 0.0f;
  int i = 0;
  float a = 0.0f;
  f = x_6.x_GLF_uniform_float_values[1].el;
  i = x_9.x_GLF_uniform_int_values[1].el;
  {
    while(true) {
      if ((i > x_9.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      a = (1.0f - max(1.0f, float(i)));
      f = min(max(a, 0.0f), 0.0f);
      {
        i = (i - 1);
      }
      continue;
    }
  }
  if ((f == x_6.x_GLF_uniform_float_values[0].el)) {
    float v = float(x_9.x_GLF_uniform_int_values[2].el);
    float v_1 = f;
    x_GLF_color = vec4(v, v_1, float(x_9.x_GLF_uniform_int_values[0].el), 1.0f);
  } else {
    x_GLF_color = vec4(0.0f);
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
