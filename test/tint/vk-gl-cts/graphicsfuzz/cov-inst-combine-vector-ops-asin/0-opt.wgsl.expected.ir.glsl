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
  float a = 0.0f;
  int i = 0;
  float b = 0.0f;
  float c = 0.0f;
  float d = 0.0f;
  bool x_67 = false;
  bool x_68_phi = false;
  float x_37 = x_6.x_GLF_uniform_float_values[0].el;
  a = x_37;
  int x_39 = x_9.x_GLF_uniform_int_values[1].el;
  i = x_39;
  {
    while(true) {
      int x_44 = i;
      int x_46 = x_9.x_GLF_uniform_int_values[0].el;
      if ((x_44 < x_46)) {
      } else {
        break;
      }
      float x_49 = a;
      b = x_49;
      float x_50 = b;
      c = x_50;
      float x_51 = c;
      d = asin(x_51);
      float x_54 = x_6.x_GLF_uniform_float_values[1].el;
      c = x_54;
      float x_55 = d;
      a = x_55;
      {
        int x_56 = i;
        i = (x_56 + 1);
      }
      continue;
    }
  }
  float x_59 = x_6.x_GLF_uniform_float_values[2].el;
  float x_60 = a;
  bool x_61 = (x_59 < x_60);
  x_68_phi = x_61;
  if (x_61) {
    float x_64 = a;
    float x_66 = x_6.x_GLF_uniform_float_values[3].el;
    x_67 = (x_64 < x_66);
    x_68_phi = x_67;
  }
  bool x_68 = x_68_phi;
  if (x_68) {
    int x_73 = x_9.x_GLF_uniform_int_values[2].el;
    int x_76 = x_9.x_GLF_uniform_int_values[1].el;
    int x_79 = x_9.x_GLF_uniform_int_values[1].el;
    int x_82 = x_9.x_GLF_uniform_int_values[2].el;
    float v = float(x_73);
    float v_1 = float(x_76);
    float v_2 = float(x_79);
    x_GLF_color = vec4(v, v_1, v_2, float(x_82));
  } else {
    int x_86 = x_9.x_GLF_uniform_int_values[1].el;
    float x_87 = float(x_86);
    x_GLF_color = vec4(x_87, x_87, x_87, x_87);
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
