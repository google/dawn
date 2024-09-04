SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[3];
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
  int x_26 = x_6.x_GLF_uniform_int_values[2].el;
  a = x_26;
  int x_28 = x_6.x_GLF_uniform_int_values[2].el;
  i = x_28;
  {
    while(true) {
      int x_33 = i;
      int x_35 = x_6.x_GLF_uniform_int_values[0].el;
      if ((x_33 < x_35)) {
      } else {
        break;
      }
      int x_38 = i;
      switch(x_38) {
        case 0:
        case -1:
        {
          int x_42 = x_6.x_GLF_uniform_int_values[1].el;
          a = x_42;
          break;
        }
        default:
        {
          break;
        }
      }
      {
        int x_43 = i;
        i = (x_43 + 1);
      }
      continue;
    }
  }
  int x_45 = a;
  int x_47 = x_6.x_GLF_uniform_int_values[1].el;
  if ((x_45 == x_47)) {
    int x_53 = x_6.x_GLF_uniform_int_values[1].el;
    int x_56 = x_6.x_GLF_uniform_int_values[2].el;
    int x_59 = x_6.x_GLF_uniform_int_values[2].el;
    int x_62 = x_6.x_GLF_uniform_int_values[1].el;
    float v = float(x_53);
    float v_1 = float(x_56);
    float v_2 = float(x_59);
    x_GLF_color = vec4(v, v_1, v_2, float(x_62));
  } else {
    int x_66 = x_6.x_GLF_uniform_int_values[2].el;
    float x_67 = float(x_66);
    x_GLF_color = vec4(x_67, x_67, x_67, x_67);
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
