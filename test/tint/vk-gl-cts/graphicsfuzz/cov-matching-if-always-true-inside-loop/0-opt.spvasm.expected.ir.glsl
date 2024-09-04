SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[5];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_8;
vec4 x_GLF_color = vec4(0.0f);
int func_i1_(inout int x) {
  int a = 0;
  int i = 0;
  int indexable[4] = int[4](0, 0, 0, 0);
  a = x_8.x_GLF_uniform_int_values[0].el;
  i = x_8.x_GLF_uniform_int_values[0].el;
  {
    while(true) {
      if ((i < x_8.x_GLF_uniform_int_values[1].el)) {
      } else {
        break;
      }
      int x_93 = a;
      indexable = int[4](x_8.x_GLF_uniform_int_values[3].el, x_8.x_GLF_uniform_int_values[3].el, x_8.x_GLF_uniform_int_values[3].el, x_8.x_GLF_uniform_int_values[3].el);
      if ((indexable[x_93] > x)) {
        if (true) {
          int x_105 = x_8.x_GLF_uniform_int_values[3].el;
          return x_105;
        } else {
          a = x_8.x_GLF_uniform_int_values[3].el;
        }
      } else {
        if (true) {
          int x_111 = x_8.x_GLF_uniform_int_values[4].el;
          return x_111;
        }
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  int x_115 = x_8.x_GLF_uniform_int_values[0].el;
  return x_115;
}
void main_1() {
  int a_1 = 0;
  int param = 0;
  int param_1 = 0;
  param = x_8.x_GLF_uniform_int_values[0].el;
  int x_40 = func_i1_(param);
  a_1 = x_40;
  param_1 = x_8.x_GLF_uniform_int_values[3].el;
  int x_43 = func_i1_(param_1);
  a_1 = (a_1 + x_43);
  if ((a_1 == x_8.x_GLF_uniform_int_values[2].el)) {
    float v = float(x_8.x_GLF_uniform_int_values[3].el);
    float v_1 = float(x_8.x_GLF_uniform_int_values[0].el);
    float v_2 = float(x_8.x_GLF_uniform_int_values[0].el);
    x_GLF_color = vec4(v, v_1, v_2, float(x_8.x_GLF_uniform_int_values[3].el));
  } else {
    x_GLF_color = vec4(float(x_8.x_GLF_uniform_int_values[0].el));
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
