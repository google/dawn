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


int x_GLF_global_loop_count = 0;
uniform buf0 x_7;
uniform buf1 x_10;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float f = 0.0f;
  int i = 0;
  x_GLF_global_loop_count = 0;
  f = x_7.x_GLF_uniform_float_values[1].el;
  i = x_10.x_GLF_uniform_int_values[1].el;
  {
    while(true) {
      if ((i < x_10.x_GLF_uniform_int_values[2].el)) {
      } else {
        break;
      }
      if ((f > x_7.x_GLF_uniform_float_values[1].el)) {
      }
      f = 1.0f;
      float v = (1.0f - clamp(x_7.x_GLF_uniform_float_values[2].el, 1.0f, f));
      f = (v + float(i));
      {
        i = (i + 1);
      }
      continue;
    }
  }
  if ((f == x_7.x_GLF_uniform_float_values[0].el)) {
    float v_1 = float(x_10.x_GLF_uniform_int_values[0].el);
    float v_2 = float(x_10.x_GLF_uniform_int_values[1].el);
    float v_3 = float(x_10.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v_1, v_2, v_3, float(x_10.x_GLF_uniform_int_values[0].el));
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
