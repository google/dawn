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
  int b = 0;
  int c = 0;
  bool x_65 = false;
  bool x_66_phi = false;
  int x_29 = x_6.x_GLF_uniform_int_values[0].el;
  a = x_29;
  int x_31 = x_6.x_GLF_uniform_int_values[1].el;
  b = x_31;
  int x_33 = x_6.x_GLF_uniform_int_values[2].el;
  c = x_33;
  {
    while(true) {
      int x_38 = a;
      int x_39 = b;
      if ((x_38 < x_39)) {
      } else {
        break;
      }
      int x_42 = a;
      a = (x_42 + 1);
      int x_44 = c;
      int x_46 = x_6.x_GLF_uniform_int_values[2].el;
      if ((x_44 == x_46)) {
        int x_52 = x_6.x_GLF_uniform_int_values[3].el;
        int x_53 = c;
        c = (x_53 * x_52);
      } else {
        if (true) {
          {
          }
          continue;
        }
      }
      {
      }
      continue;
    }
  }
  int x_57 = a;
  int x_58 = b;
  bool x_59 = (x_57 == x_58);
  x_66_phi = x_59;
  if (x_59) {
    int x_62 = c;
    int x_64 = x_6.x_GLF_uniform_int_values[3].el;
    x_65 = (x_62 == x_64);
    x_66_phi = x_65;
  }
  bool x_66 = x_66_phi;
  if (x_66) {
    int x_71 = x_6.x_GLF_uniform_int_values[2].el;
    int x_74 = x_6.x_GLF_uniform_int_values[0].el;
    int x_77 = x_6.x_GLF_uniform_int_values[0].el;
    int x_80 = x_6.x_GLF_uniform_int_values[2].el;
    float v = float(x_71);
    float v_1 = float(x_74);
    float v_2 = float(x_77);
    x_GLF_color = vec4(v, v_1, v_2, float(x_80));
  } else {
    int x_84 = x_6.x_GLF_uniform_int_values[0].el;
    float x_85 = float(x_84);
    x_GLF_color = vec4(x_85, x_85, x_85, x_85);
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
