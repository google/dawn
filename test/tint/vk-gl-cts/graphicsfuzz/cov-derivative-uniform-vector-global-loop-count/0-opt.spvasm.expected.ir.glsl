SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[2];
};

struct strided_arr_1 {
  int el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_int_values[3];
};

struct buf2 {
  vec2 injectionSwitch;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


int x_GLF_global_loop_count = 0;
uniform buf1 x_7;
uniform buf0 x_10;
uniform buf2 x_12;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float f = 0.0f;
  int r = 0;
  x_GLF_global_loop_count = 0;
  f = x_7.x_GLF_uniform_float_values[0].el;
  r = x_10.x_GLF_uniform_int_values[1].el;
  {
    while(true) {
      if ((r < x_10.x_GLF_uniform_int_values[2].el)) {
      } else {
        break;
      }
      x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
      vec2 x_57 = x_12.injectionSwitch;
      float x_60 = f;
      f = (x_60 + dFdx(x_57)[1u]);
      {
        r = (r + 1);
      }
      continue;
    }
  }
  {
    while(true) {
      if ((x_GLF_global_loop_count < 100)) {
      } else {
        break;
      }
      x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
      f = (f + x_7.x_GLF_uniform_float_values[0].el);
      {
      }
      continue;
    }
  }
  if ((f == x_7.x_GLF_uniform_float_values[1].el)) {
    float v = float(x_10.x_GLF_uniform_int_values[0].el);
    float v_1 = float(x_10.x_GLF_uniform_int_values[1].el);
    float v_2 = float(x_10.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v, v_1, v_2, float(x_10.x_GLF_uniform_int_values[0].el));
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
