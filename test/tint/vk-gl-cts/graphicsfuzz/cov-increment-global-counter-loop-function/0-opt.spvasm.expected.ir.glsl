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
  int x_66 = 0;
  int x_62 = x_7.x_GLF_uniform_int_values[1].el;
  x_66 = x_7.x_GLF_uniform_int_values[0].el;
  {
    while(true) {
      int x_67 = 0;
      if ((x_66 < x_7.x_GLF_uniform_int_values[3].el)) {
      } else {
        break;
      }
      {
        x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
        x_67 = (x_66 + 1);
        x_66 = x_67;
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
      if ((x_GLF_global_loop_count < 10)) {
      } else {
        break;
      }
      {
        x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
        func_();
      }
      continue;
    }
  }
  {
    while(true) {
      if ((x_GLF_global_loop_count < 10)) {
      } else {
        break;
      }
      {
        x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
      }
      continue;
    }
  }
  if ((x_GLF_global_loop_count == x_7.x_GLF_uniform_int_values[2].el)) {
    float x_51 = float(x_7.x_GLF_uniform_int_values[1].el);
    float x_54 = float(x_7.x_GLF_uniform_int_values[0].el);
    x_GLF_color = vec4(x_51, x_54, x_54, x_51);
  } else {
    x_GLF_color = vec4(float(x_7.x_GLF_uniform_int_values[0].el));
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
