SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[3];
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
uniform buf1 x_10;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float f0 = 0.0f;
  float f1 = 0.0f;
  int i = 0;
  bool x_63 = false;
  bool x_64_phi = false;
  float x_34 = x_6.x_GLF_uniform_float_values[0].el;
  f0 = x_34;
  float x_36 = x_6.x_GLF_uniform_float_values[0].el;
  f1 = x_36;
  int x_38 = x_10.x_GLF_uniform_int_values[1].el;
  i = x_38;
  {
    while(true) {
      int x_43 = i;
      int x_45 = x_10.x_GLF_uniform_int_values[0].el;
      if ((x_43 < x_45)) {
      } else {
        break;
      }
      float x_48 = f0;
      f0 = abs((1.10000002384185791016f * x_48));
      float x_51 = f0;
      f1 = x_51;
      {
        int x_52 = i;
        i = (x_52 + 1);
      }
      continue;
    }
  }
  float x_54 = f1;
  float x_56 = x_6.x_GLF_uniform_float_values[1].el;
  bool x_57 = (x_54 > x_56);
  x_64_phi = x_57;
  if (x_57) {
    float x_60 = f1;
    float x_62 = x_6.x_GLF_uniform_float_values[2].el;
    x_63 = (x_60 < x_62);
    x_64_phi = x_63;
  }
  bool x_64 = x_64_phi;
  if (x_64) {
    int x_69 = x_10.x_GLF_uniform_int_values[2].el;
    int x_72 = x_10.x_GLF_uniform_int_values[1].el;
    int x_75 = x_10.x_GLF_uniform_int_values[1].el;
    int x_78 = x_10.x_GLF_uniform_int_values[2].el;
    float v = float(x_69);
    float v_1 = float(x_72);
    float v_2 = float(x_75);
    x_GLF_color = vec4(v, v_1, v_2, float(x_78));
  } else {
    int x_82 = x_10.x_GLF_uniform_int_values[1].el;
    float x_83 = float(x_82);
    x_GLF_color = vec4(x_83, x_83, x_83, x_83);
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
