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
  float x_36 = x_7.x_GLF_uniform_float_values[1].el;
  f = x_36;
  int x_38 = x_10.x_GLF_uniform_int_values[1].el;
  i = x_38;
  {
    while(true) {
      int x_43 = i;
      int x_45 = x_10.x_GLF_uniform_int_values[2].el;
      if ((x_43 < x_45)) {
      } else {
        break;
      }
      float x_48 = f;
      float x_50 = x_7.x_GLF_uniform_float_values[1].el;
      if ((x_48 > x_50)) {
      }
      f = 1.0f;
      float x_55 = x_7.x_GLF_uniform_float_values[2].el;
      float x_56 = f;
      int x_59 = i;
      float v = (1.0f - clamp(x_55, 1.0f, x_56));
      f = (v + float(x_59));
      {
        int x_62 = i;
        i = (x_62 + 1);
      }
      continue;
    }
  }
  float x_64 = f;
  float x_66 = x_7.x_GLF_uniform_float_values[0].el;
  if ((x_64 == x_66)) {
    int x_72 = x_10.x_GLF_uniform_int_values[0].el;
    int x_75 = x_10.x_GLF_uniform_int_values[1].el;
    int x_78 = x_10.x_GLF_uniform_int_values[1].el;
    int x_81 = x_10.x_GLF_uniform_int_values[0].el;
    float v_1 = float(x_72);
    float v_2 = float(x_75);
    float v_3 = float(x_78);
    x_GLF_color = vec4(v_1, v_2, v_3, float(x_81));
  } else {
    int x_85 = x_10.x_GLF_uniform_int_values[1].el;
    float x_86 = float(x_85);
    x_GLF_color = vec4(x_86, x_86, x_86, x_86);
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
