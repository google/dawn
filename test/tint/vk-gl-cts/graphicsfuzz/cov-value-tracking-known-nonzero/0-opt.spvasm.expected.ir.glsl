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


uniform buf0 x_7;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int a = 0;
  int sum = 0;
  int i = 0;
  a = 65536;
  sum = x_7.x_GLF_uniform_int_values[0].el;
  if ((1 == x_7.x_GLF_uniform_int_values[1].el)) {
    a = (a - 1);
  }
  i = 0;
  {
    while(true) {
      if ((i < a)) {
      } else {
        break;
      }
      sum = (sum + i);
      {
        i = (i + x_7.x_GLF_uniform_int_values[2].el);
      }
      continue;
    }
  }
  if ((sum == x_7.x_GLF_uniform_int_values[3].el)) {
    float v = float(x_7.x_GLF_uniform_int_values[1].el);
    float v_1 = float(x_7.x_GLF_uniform_int_values[0].el);
    float v_2 = float(x_7.x_GLF_uniform_int_values[0].el);
    x_GLF_color = vec4(v, v_1, v_2, float(x_7.x_GLF_uniform_int_values[1].el));
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
