SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[4];
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


uniform buf0 x_6;
uniform buf1 x_9;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float arr[3] = float[3](0.0f, 0.0f, 0.0f);
  int i = 0;
  arr = float[3](x_6.x_GLF_uniform_float_values[0].el, x_6.x_GLF_uniform_float_values[1].el, x_6.x_GLF_uniform_float_values[2].el);
  i = 1;
  {
    while(true) {
      int v = i;
      if ((v < min(x_9.x_GLF_uniform_int_values[2].el, 3))) {
      } else {
        break;
      }
      int x_53 = x_9.x_GLF_uniform_int_values[2].el;
      arr[x_53] = (arr[x_53] + x_6.x_GLF_uniform_float_values[0].el);
      {
        i = (i + 1);
      }
      continue;
    }
  }
  if ((arr[x_9.x_GLF_uniform_int_values[2].el] == x_6.x_GLF_uniform_float_values[3].el)) {
    float v_1 = float(x_9.x_GLF_uniform_int_values[1].el);
    float v_2 = float(x_9.x_GLF_uniform_int_values[0].el);
    float v_3 = float(x_9.x_GLF_uniform_int_values[0].el);
    x_GLF_color = vec4(v_1, v_2, v_3, float(x_9.x_GLF_uniform_int_values[1].el));
  } else {
    x_GLF_color = vec4(float(x_9.x_GLF_uniform_int_values[0].el));
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
