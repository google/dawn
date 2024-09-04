SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf1 {
  strided_arr x_GLF_uniform_int_values[2];
};

struct strided_arr_1 {
  float el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_float_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
uniform buf1 x_5;
uniform buf0 x_8;
void main_1() {
  int i = 0;
  x_GLF_color = vec4(float(x_5.x_GLF_uniform_int_values[0].el));
  i = x_5.x_GLF_uniform_int_values[0].el;
  {
    while(true) {
      if ((i < x_5.x_GLF_uniform_int_values[1].el)) {
      } else {
        break;
      }
      float v = x_8.x_GLF_uniform_float_values[1].el;
      if (!((v <= float(i)))) {
        vec4 v_1 = x_GLF_color;
        float v_2 = x_8.x_GLF_uniform_float_values[0].el;
        float v_3 = float(i);
        float v_4 = float(i);
        x_GLF_color = (v_1 + vec4(v_2, v_3, v_4, x_8.x_GLF_uniform_float_values[0].el));
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
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
