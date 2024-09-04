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
  int x_29 = x_7.x_GLF_uniform_int_values[0].el;
  sum = x_29;
  int x_31 = x_7.x_GLF_uniform_int_values[1].el;
  if ((1 == x_31)) {
    int x_35 = a;
    a = (x_35 - 1);
  }
  i = 0;
  {
    while(true) {
      int x_41 = i;
      int x_42 = a;
      if ((x_41 < x_42)) {
      } else {
        break;
      }
      int x_45 = i;
      int x_46 = sum;
      sum = (x_46 + x_45);
      {
        int x_49 = x_7.x_GLF_uniform_int_values[2].el;
        int x_50 = i;
        i = (x_50 + x_49);
      }
      continue;
    }
  }
  int x_52 = sum;
  int x_54 = x_7.x_GLF_uniform_int_values[3].el;
  if ((x_52 == x_54)) {
    int x_60 = x_7.x_GLF_uniform_int_values[1].el;
    int x_63 = x_7.x_GLF_uniform_int_values[0].el;
    int x_66 = x_7.x_GLF_uniform_int_values[0].el;
    int x_69 = x_7.x_GLF_uniform_int_values[1].el;
    float v = float(x_60);
    float v_1 = float(x_63);
    float v_2 = float(x_66);
    x_GLF_color = vec4(v, v_1, v_2, float(x_69));
  } else {
    int x_73 = x_7.x_GLF_uniform_int_values[0].el;
    float x_74 = float(x_73);
    x_GLF_color = vec4(x_74, x_74, x_74, x_74);
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
