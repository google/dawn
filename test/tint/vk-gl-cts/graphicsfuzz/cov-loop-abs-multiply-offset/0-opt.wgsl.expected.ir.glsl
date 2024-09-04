SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[4];
};

struct strided_arr_1 {
  int el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_int_values[3];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
uniform buf1 x_9;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float f = 0.0f;
  int i = 0;
  bool x_66 = false;
  bool x_67_phi = false;
  float x_34 = x_6.x_GLF_uniform_float_values[0].el;
  f = x_34;
  int x_36 = x_9.x_GLF_uniform_int_values[1].el;
  i = x_36;
  {
    while(true) {
      int x_41 = i;
      int x_43 = x_9.x_GLF_uniform_int_values[0].el;
      if ((x_41 < x_43)) {
      } else {
        break;
      }
      float x_47 = x_6.x_GLF_uniform_float_values[3].el;
      float x_49 = f;
      float x_53 = x_6.x_GLF_uniform_float_values[0].el;
      f = (abs((-(x_47) * x_49)) + x_53);
      {
        int x_55 = i;
        i = (x_55 + 1);
      }
      continue;
    }
  }
  float x_57 = f;
  float x_59 = x_6.x_GLF_uniform_float_values[1].el;
  bool x_60 = (x_57 > x_59);
  x_67_phi = x_60;
  if (x_60) {
    float x_63 = f;
    float x_65 = x_6.x_GLF_uniform_float_values[2].el;
    x_66 = (x_63 < x_65);
    x_67_phi = x_66;
  }
  bool x_67 = x_67_phi;
  if (x_67) {
    int x_72 = x_9.x_GLF_uniform_int_values[2].el;
    int x_75 = x_9.x_GLF_uniform_int_values[1].el;
    int x_78 = x_9.x_GLF_uniform_int_values[1].el;
    int x_81 = x_9.x_GLF_uniform_int_values[2].el;
    float v = float(x_72);
    float v_1 = float(x_75);
    float v_2 = float(x_78);
    x_GLF_color = vec4(v, v_1, v_2, float(x_81));
  } else {
    int x_85 = x_9.x_GLF_uniform_int_values[1].el;
    float x_86 = float(x_85);
    x_GLF_color = vec4(x_86, x_86, x_86, x_86);
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
