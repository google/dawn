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
  float v = float(x_6.x_GLF_uniform_int_values[1].el);
  float v_1 = float(x_6.x_GLF_uniform_int_values[0].el);
  float v_2 = float(x_6.x_GLF_uniform_int_values[0].el);
  x_GLF_color = vec4(v, v_1, v_2, float(x_6.x_GLF_uniform_int_values[1].el));
  {
    while(true) {
      bool x_54 = false;
      bool x_55 = false;
      if ((x_GLF_global_loop_count < 100)) {
      } else {
        break;
      }
      x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
      x_55 = true;
      if (false) {
        x_54 = (x_6.x_GLF_uniform_int_values[0].el == x_6.x_GLF_uniform_int_values[1].el);
        x_55 = x_54;
      }
      if (!(x_55)) {
        break;
      }
      {
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
      x_GLF_color = vec4(float(x_6.x_GLF_uniform_int_values[0].el));
      {
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
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
