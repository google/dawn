SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[4];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


int x_GLF_global_loop_count = 0;
uniform buf0 x_7;
vec4 x_GLF_color = vec4(0.0f);
void func_() {
  int x_66_phi = 0;
  int x_62 = x_7.x_GLF_uniform_int_values[1].el;
  int x_64 = x_7.x_GLF_uniform_int_values[0].el;
  x_66_phi = x_64;
  {
    while(true) {
      int x_67 = 0;
      int x_66 = x_66_phi;
      int x_70 = x_7.x_GLF_uniform_int_values[3].el;
      if ((x_66 < x_70)) {
      } else {
        break;
      }
      {
        int x_73 = x_GLF_global_loop_count;
        x_GLF_global_loop_count = (x_73 + 1);
        x_67 = (x_66 + 1);
        x_66_phi = x_67;
      }
      continue;
    }
  }
  if ((x_62 < x_62)) {
    return;
  }
}
void main_1() {
  x_GLF_global_loop_count = 0;
  {
    while(true) {
      int x_28 = x_GLF_global_loop_count;
      if ((x_28 < 10)) {
      } else {
        break;
      }
      {
        int x_32 = x_GLF_global_loop_count;
        x_GLF_global_loop_count = (x_32 + 1);
        func_();
      }
      continue;
    }
  }
  {
    while(true) {
      int x_36 = x_GLF_global_loop_count;
      if ((x_36 < 10)) {
      } else {
        break;
      }
      {
        int x_40 = x_GLF_global_loop_count;
        x_GLF_global_loop_count = (x_40 + 1);
      }
      continue;
    }
  }
  int x_42 = x_GLF_global_loop_count;
  int x_44 = x_7.x_GLF_uniform_int_values[2].el;
  if ((x_42 == x_44)) {
    int x_50 = x_7.x_GLF_uniform_int_values[1].el;
    float x_51 = float(x_50);
    int x_53 = x_7.x_GLF_uniform_int_values[0].el;
    float x_54 = float(x_53);
    x_GLF_color = vec4(x_51, x_54, x_54, x_51);
  } else {
    int x_57 = x_7.x_GLF_uniform_int_values[0].el;
    float x_58 = float(x_57);
    x_GLF_color = vec4(x_58, x_58, x_58, x_58);
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
