SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


int x_GLF_global_loop_count = 0;
vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_6;
void main_1() {
  x_GLF_global_loop_count = 0;
  {
    while(true) {
      int x_30 = x_GLF_global_loop_count;
      if ((x_30 < 100)) {
      } else {
        break;
      }
      int x_33 = x_GLF_global_loop_count;
      x_GLF_global_loop_count = (x_33 + 1);
      int x_35 = x_GLF_global_loop_count;
      int x_36 = x_GLF_global_loop_count;
      if (((x_35 * x_36) > 10)) {
        break;
      }
      {
      }
      continue;
    }
  }
  int x_41 = x_GLF_global_loop_count;
  if ((x_41 == 4)) {
    int x_47 = x_6.x_GLF_uniform_int_values[0].el;
    int x_50 = x_6.x_GLF_uniform_int_values[1].el;
    int x_53 = x_6.x_GLF_uniform_int_values[1].el;
    int x_56 = x_6.x_GLF_uniform_int_values[0].el;
    float v = float(x_47);
    float v_1 = float(x_50);
    float v_2 = float(x_53);
    x_GLF_color = vec4(v, v_1, v_2, float(x_56));
  } else {
    int x_60 = x_6.x_GLF_uniform_int_values[1].el;
    float x_61 = float(x_60);
    x_GLF_color = vec4(x_61, x_61, x_61, x_61);
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
