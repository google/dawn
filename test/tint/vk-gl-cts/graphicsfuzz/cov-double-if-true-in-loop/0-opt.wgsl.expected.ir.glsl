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


uniform buf0 x_7;
vec4 x_GLF_color = vec4(0.0f);
int func_() {
  int i = 0;
  int x_53 = x_7.x_GLF_uniform_int_values[0].el;
  i = x_53;
  {
    while(true) {
      int x_58 = i;
      i = (x_58 + 1);
      if (true) {
        if (true) {
          int x_65 = x_7.x_GLF_uniform_int_values[2].el;
          return x_65;
        }
      }
      {
        int x_66 = i;
        int x_68 = x_7.x_GLF_uniform_int_values[1].el;
        if (!((x_66 < x_68))) { break; }
      }
      continue;
    }
  }
  int x_71 = x_7.x_GLF_uniform_int_values[0].el;
  return x_71;
}
void main_1() {
  int x_27 = func_();
  int x_29 = x_7.x_GLF_uniform_int_values[2].el;
  if ((x_27 == x_29)) {
    int x_35 = x_7.x_GLF_uniform_int_values[2].el;
    int x_38 = x_7.x_GLF_uniform_int_values[0].el;
    int x_41 = x_7.x_GLF_uniform_int_values[0].el;
    int x_44 = x_7.x_GLF_uniform_int_values[2].el;
    float v = float(x_35);
    float v_1 = float(x_38);
    float v_2 = float(x_41);
    x_GLF_color = vec4(v, v_1, v_2, float(x_44));
  } else {
    int x_48 = x_7.x_GLF_uniform_int_values[0].el;
    float x_49 = float(x_48);
    x_GLF_color = vec4(x_49, x_49, x_49, x_49);
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
