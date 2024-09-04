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
  float x_42 = x_7.x_GLF_uniform_float_values[0].el;
  f = x_42;
  int x_44 = x_10.x_GLF_uniform_int_values[1].el;
  r = x_44;
  {
    while(true) {
      int x_49 = r;
      int x_51 = x_10.x_GLF_uniform_int_values[2].el;
      if ((x_49 < x_51)) {
      } else {
        break;
      }
      int x_54 = x_GLF_global_loop_count;
      x_GLF_global_loop_count = (x_54 + 1);
      vec2 x_57 = x_12.injectionSwitch;
      float x_60 = f;
      f = (x_60 + dFdx(x_57)[1u]);
      {
        int x_62 = r;
        r = (x_62 + 1);
      }
      continue;
    }
  }
  {
    while(true) {
      int x_68 = x_GLF_global_loop_count;
      if ((x_68 < 100)) {
      } else {
        break;
      }
      int x_71 = x_GLF_global_loop_count;
      x_GLF_global_loop_count = (x_71 + 1);
      float x_74 = x_7.x_GLF_uniform_float_values[0].el;
      float x_75 = f;
      f = (x_75 + x_74);
      {
      }
      continue;
    }
  }
  float x_77 = f;
  float x_79 = x_7.x_GLF_uniform_float_values[1].el;
  if ((x_77 == x_79)) {
    int x_85 = x_10.x_GLF_uniform_int_values[0].el;
    int x_88 = x_10.x_GLF_uniform_int_values[1].el;
    int x_91 = x_10.x_GLF_uniform_int_values[1].el;
    int x_94 = x_10.x_GLF_uniform_int_values[0].el;
    float v = float(x_85);
    float v_1 = float(x_88);
    float v_2 = float(x_91);
    x_GLF_color = vec4(v, v_1, v_2, float(x_94));
  } else {
    int x_98 = x_10.x_GLF_uniform_int_values[1].el;
    float x_99 = float(x_98);
    x_GLF_color = vec4(x_99, x_99, x_99, x_99);
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
