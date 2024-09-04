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
  int c = 0;
  int i = 0;
  int x_27 = x_6.x_GLF_uniform_int_values[2].el;
  c = x_27;
  int x_29 = x_6.x_GLF_uniform_int_values[2].el;
  i = x_29;
  {
    while(true) {
      int x_34 = i;
      int x_36 = x_6.x_GLF_uniform_int_values[0].el;
      if ((x_34 < x_36)) {
      } else {
        break;
      }
      int x_39 = i;
      c = ~(x_39);
      int x_41 = c;
      c = min(max(x_41, 0), 3);
      {
        int x_43 = i;
        i = (x_43 + 1);
      }
      continue;
    }
  }
  int x_46 = x_6.x_GLF_uniform_int_values[1].el;
  float x_47 = float(x_46);
  x_GLF_color = vec4(x_47, x_47, x_47, x_47);
  int x_49 = c;
  int x_51 = x_6.x_GLF_uniform_int_values[1].el;
  if ((x_49 == x_51)) {
    int x_56 = x_6.x_GLF_uniform_int_values[2].el;
    int x_59 = x_6.x_GLF_uniform_int_values[1].el;
    int x_62 = x_6.x_GLF_uniform_int_values[1].el;
    int x_65 = x_6.x_GLF_uniform_int_values[2].el;
    float v = float(x_56);
    float v_1 = float(x_59);
    float v_2 = float(x_62);
    x_GLF_color = vec4(v, v_1, v_2, float(x_65));
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
