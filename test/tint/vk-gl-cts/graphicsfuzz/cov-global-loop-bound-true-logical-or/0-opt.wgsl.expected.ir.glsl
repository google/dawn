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
  int x_26 = x_6.x_GLF_uniform_int_values[1].el;
  int x_29 = x_6.x_GLF_uniform_int_values[0].el;
  int x_32 = x_6.x_GLF_uniform_int_values[0].el;
  int x_35 = x_6.x_GLF_uniform_int_values[1].el;
  float v = float(x_26);
  float v_1 = float(x_29);
  float v_2 = float(x_32);
  x_GLF_color = vec4(v, v_1, v_2, float(x_35));
  {
    while(true) {
      bool x_54 = false;
      bool x_55_phi = false;
      int x_42 = x_GLF_global_loop_count;
      if ((x_42 < 100)) {
      } else {
        break;
      }
      int x_45 = x_GLF_global_loop_count;
      x_GLF_global_loop_count = (x_45 + 1);
      x_55_phi = true;
      if (false) {
        int x_51 = x_6.x_GLF_uniform_int_values[0].el;
        int x_53 = x_6.x_GLF_uniform_int_values[1].el;
        x_54 = (x_51 == x_53);
        x_55_phi = x_54;
      }
      bool x_55 = x_55_phi;
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
      int x_63 = x_GLF_global_loop_count;
      if ((x_63 < 100)) {
      } else {
        break;
      }
      int x_66 = x_GLF_global_loop_count;
      x_GLF_global_loop_count = (x_66 + 1);
      int x_69 = x_6.x_GLF_uniform_int_values[0].el;
      float x_70 = float(x_69);
      x_GLF_color = vec4(x_70, x_70, x_70, x_70);
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
