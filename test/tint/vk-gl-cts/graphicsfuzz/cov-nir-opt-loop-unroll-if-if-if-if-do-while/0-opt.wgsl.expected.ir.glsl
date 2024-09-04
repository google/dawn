SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[2];
};

struct strided_arr_1 {
  int el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_5;
uniform buf1 x_8;
void main_1() {
  float a = 0.0f;
  float x_31 = x_5.x_GLF_uniform_float_values[1].el;
  x_GLF_color = vec4(x_31, x_31, x_31, x_31);
  float x_34 = x_5.x_GLF_uniform_float_values[0].el;
  a = x_34;
  {
    while(true) {
      float x_40 = x_5.x_GLF_uniform_float_values[0].el;
      float x_43 = x_5.x_GLF_uniform_float_values[0].el;
      if (((x_40 / 0.20000000298023223877f) < x_43)) {
        return;
      }
      float x_48 = x_5.x_GLF_uniform_float_values[0].el;
      float x_51 = x_5.x_GLF_uniform_float_values[0].el;
      if (((x_48 / 0.20000000298023223877f) < x_51)) {
        return;
      }
      float x_56 = x_5.x_GLF_uniform_float_values[0].el;
      float x_59 = x_5.x_GLF_uniform_float_values[0].el;
      if (((x_56 / 0.20000000298023223877f) < x_59)) {
        return;
      }
      float x_64 = x_5.x_GLF_uniform_float_values[0].el;
      float x_67 = x_5.x_GLF_uniform_float_values[0].el;
      if (((x_64 / 0.20000000298023223877f) < x_67)) {
        return;
      } else {
        a = 0.0f;
      }
      {
        float x_72 = a;
        if (!((x_72 == 0.0f))) { break; }
      }
      continue;
    }
  }
  int x_75 = x_8.x_GLF_uniform_int_values[1].el;
  int x_78 = x_8.x_GLF_uniform_int_values[0].el;
  int x_81 = x_8.x_GLF_uniform_int_values[0].el;
  int x_84 = x_8.x_GLF_uniform_int_values[1].el;
  float v = float(x_75);
  float v_1 = float(x_78);
  float v_2 = float(x_81);
  x_GLF_color = vec4(v, v_1, v_2, float(x_84));
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
