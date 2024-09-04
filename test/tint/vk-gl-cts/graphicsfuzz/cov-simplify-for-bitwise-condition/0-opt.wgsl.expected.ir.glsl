SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int a = 0;
  int i = 0;
  int x_25 = x_6.x_GLF_uniform_int_values[1].el;
  a = x_25;
  int x_27 = x_6.x_GLF_uniform_int_values[0].el;
  i = -(x_27);
  {
    while(true) {
      int x_33 = i;
      int x_35 = x_6.x_GLF_uniform_int_values[0].el;
      int x_38 = x_6.x_GLF_uniform_int_values[0].el;
      if (((x_33 | x_35) < x_38)) {
      } else {
        break;
      }
      int x_41 = i;
      int x_43 = x_6.x_GLF_uniform_int_values[0].el;
      a = (x_41 * x_43);
      {
        int x_45 = i;
        i = (x_45 + 1);
      }
      continue;
    }
  }
  int x_47 = a;
  int x_49 = x_6.x_GLF_uniform_int_values[0].el;
  if ((x_47 == -(x_49))) {
    int x_56 = x_6.x_GLF_uniform_int_values[0].el;
    int x_59 = x_6.x_GLF_uniform_int_values[1].el;
    int x_62 = x_6.x_GLF_uniform_int_values[1].el;
    int x_65 = x_6.x_GLF_uniform_int_values[0].el;
    float v = float(x_56);
    float v_1 = float(x_59);
    float v_2 = float(x_62);
    x_GLF_color = vec4(v, v_1, v_2, float(x_65));
  } else {
    int x_68 = a;
    float x_69 = float(x_68);
    x_GLF_color = vec4(x_69, x_69, x_69, x_69);
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
