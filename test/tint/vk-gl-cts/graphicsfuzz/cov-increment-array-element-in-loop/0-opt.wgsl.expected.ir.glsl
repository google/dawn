SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[3];
};

struct strided_arr_1 {
  int el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_int_values[3];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf1 x_6;
uniform buf0 x_9;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float arr[3] = float[3](0.0f, 0.0f, 0.0f);
  int a = 0;
  bool x_69 = false;
  bool x_79 = false;
  bool x_70_phi = false;
  bool x_80_phi = false;
  float x_34 = x_6.x_GLF_uniform_float_values[1].el;
  float x_36 = x_6.x_GLF_uniform_float_values[0].el;
  float x_38 = x_6.x_GLF_uniform_float_values[2].el;
  arr = float[3](x_34, x_36, x_38);
  a = 0;
  {
    while(true) {
      int x_44 = a;
      int x_46 = x_9.x_GLF_uniform_int_values[1].el;
      if ((x_44 <= x_46)) {
      } else {
        break;
      }
      int x_49 = a;
      a = (x_49 + 1);
      float x_52 = x_6.x_GLF_uniform_float_values[0].el;
      arr[x_49] = x_52;
      {
      }
      continue;
    }
  }
  int x_55 = x_9.x_GLF_uniform_int_values[1].el;
  float x_57 = arr[x_55];
  float x_59 = x_6.x_GLF_uniform_float_values[0].el;
  bool x_60 = (x_57 == x_59);
  x_70_phi = x_60;
  if (x_60) {
    int x_64 = x_9.x_GLF_uniform_int_values[2].el;
    float x_66 = arr[x_64];
    float x_68 = x_6.x_GLF_uniform_float_values[0].el;
    x_69 = (x_66 == x_68);
    x_70_phi = x_69;
  }
  bool x_70 = x_70_phi;
  x_80_phi = x_70;
  if (x_70) {
    int x_74 = x_9.x_GLF_uniform_int_values[0].el;
    float x_76 = arr[x_74];
    float x_78 = x_6.x_GLF_uniform_float_values[2].el;
    x_79 = (x_76 == x_78);
    x_80_phi = x_79;
  }
  bool x_80 = x_80_phi;
  if (x_80) {
    int x_85 = x_9.x_GLF_uniform_int_values[1].el;
    float x_87 = arr[x_85];
    float x_89 = x_6.x_GLF_uniform_float_values[1].el;
    float x_91 = x_6.x_GLF_uniform_float_values[1].el;
    float x_93 = x_6.x_GLF_uniform_float_values[0].el;
    x_GLF_color = vec4(x_87, x_89, x_91, x_93);
  } else {
    float x_96 = x_6.x_GLF_uniform_float_values[1].el;
    x_GLF_color = vec4(x_96, x_96, x_96, x_96);
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
