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
  int i = 0;
  int A[2] = int[2](0, 0);
  int a = 0;
  int x_30 = x_6.x_GLF_uniform_int_values[1].el;
  i = x_30;
  {
    while(true) {
      int x_35 = i;
      int x_37 = x_6.x_GLF_uniform_int_values[0].el;
      if ((x_35 < x_37)) {
      } else {
        break;
      }
      int x_40 = i;
      int x_41 = i;
      A[x_40] = x_41;
      {
        int x_43 = i;
        i = (x_43 + 1);
      }
      continue;
    }
  }
  int x_46 = x_6.x_GLF_uniform_int_values[1].el;
  int x_48 = A[x_46];
  int x_51 = x_6.x_GLF_uniform_int_values[2].el;
  int x_53 = A[x_51];
  a = min(~(x_48), ~(x_53));
  int x_57 = x_6.x_GLF_uniform_int_values[1].el;
  float x_58 = float(x_57);
  x_GLF_color = vec4(x_58, x_58, x_58, x_58);
  int x_60 = a;
  int x_62 = x_6.x_GLF_uniform_int_values[0].el;
  if ((x_60 == -(x_62))) {
    int x_68 = x_6.x_GLF_uniform_int_values[2].el;
    int x_71 = x_6.x_GLF_uniform_int_values[1].el;
    int x_74 = x_6.x_GLF_uniform_int_values[1].el;
    int x_77 = x_6.x_GLF_uniform_int_values[2].el;
    float v = float(x_68);
    float v_1 = float(x_71);
    float v_2 = float(x_74);
    x_GLF_color = vec4(v, v_1, v_2, float(x_77));
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
