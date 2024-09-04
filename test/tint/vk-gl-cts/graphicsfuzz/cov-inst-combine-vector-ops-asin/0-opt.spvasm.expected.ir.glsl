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
  float a = 0.0f;
  int i = 0;
  float b = 0.0f;
  float c = 0.0f;
  float d = 0.0f;
  bool x_67 = false;
  bool x_68 = false;
  a = x_6.x_GLF_uniform_float_values[0].el;
  i = x_9.x_GLF_uniform_int_values[1].el;
  {
    while(true) {
      if ((i < x_9.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      b = a;
      c = b;
      d = asin(c);
      c = x_6.x_GLF_uniform_float_values[1].el;
      a = d;
      {
        i = (i + 1);
      }
      continue;
    }
  }
  bool x_61 = (x_6.x_GLF_uniform_float_values[2].el < a);
  x_68 = x_61;
  if (x_61) {
    x_67 = (a < x_6.x_GLF_uniform_float_values[3].el);
    x_68 = x_67;
  }
  if (x_68) {
    float v = float(x_9.x_GLF_uniform_int_values[2].el);
    float v_1 = float(x_9.x_GLF_uniform_int_values[1].el);
    float v_2 = float(x_9.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v, v_1, v_2, float(x_9.x_GLF_uniform_int_values[2].el));
  } else {
    x_GLF_color = vec4(float(x_9.x_GLF_uniform_int_values[1].el));
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
