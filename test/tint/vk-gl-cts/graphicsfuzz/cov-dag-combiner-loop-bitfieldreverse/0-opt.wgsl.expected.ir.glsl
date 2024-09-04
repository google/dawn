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


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int a = 0;
  int i = 0;
  int x_27 = x_6.x_GLF_uniform_int_values[1].el;
  a = x_27;
  int x_29 = x_6.x_GLF_uniform_int_values[3].el;
  i = -(x_29);
  {
    while(true) {
      int x_35 = i;
      int x_36 = (x_35 + 1);
      i = x_36;
      int x_39 = x_6.x_GLF_uniform_int_values[2].el;
      if ((bitfieldReverse(x_36) <= x_39)) {
      } else {
        break;
      }
      int x_42 = a;
      a = (x_42 + 1);
      {
      }
      continue;
    }
  }
  int x_44 = a;
  int x_46 = x_6.x_GLF_uniform_int_values[0].el;
  if ((x_44 == x_46)) {
    int x_52 = x_6.x_GLF_uniform_int_values[2].el;
    int x_55 = x_6.x_GLF_uniform_int_values[1].el;
    int x_58 = x_6.x_GLF_uniform_int_values[1].el;
    int x_61 = x_6.x_GLF_uniform_int_values[2].el;
    float v = float(x_52);
    float v_1 = float(x_55);
    float v_2 = float(x_58);
    x_GLF_color = vec4(v, v_1, v_2, float(x_61));
  } else {
    int x_65 = x_6.x_GLF_uniform_int_values[1].el;
    float x_66 = float(x_65);
    x_GLF_color = vec4(x_66, x_66, x_66, x_66);
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
