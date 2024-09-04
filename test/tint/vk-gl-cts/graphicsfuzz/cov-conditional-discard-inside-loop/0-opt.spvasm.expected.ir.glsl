SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[3];
};

struct buf2 {
  vec2 injectionSwitch;
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


vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_6;
uniform buf2 x_9;
uniform buf1 x_11;
bool continue_execution = true;
void main_1() {
  int a = 0;
  int i = 0;
  a = 1;
  float v = float(x_6.x_GLF_uniform_int_values[0].el);
  float v_1 = float(x_6.x_GLF_uniform_int_values[1].el);
  float v_2 = float(x_6.x_GLF_uniform_int_values[1].el);
  x_GLF_color = vec4(v, v_1, v_2, float(x_6.x_GLF_uniform_int_values[0].el));
  i = x_6.x_GLF_uniform_int_values[1].el;
  {
    while(true) {
      if ((i < x_6.x_GLF_uniform_int_values[2].el)) {
      } else {
        break;
      }
      int x_61 = a;
      a = (a + 1);
      if ((x_61 > 3)) {
        break;
      }
      if ((x_9.injectionSwitch.x > x_11.x_GLF_uniform_float_values[0].el)) {
        continue_execution = false;
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
  main_out v_3 = main_out(x_GLF_color);
  if (!(continue_execution)) {
    discard;
  }
  return v_3;
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
