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
  float arr[3] = float[3](0.0f, 0.0f, 0.0f);
  int i = 0;
  float x_36 = x_6.x_GLF_uniform_float_values[0].el;
  float x_38 = x_6.x_GLF_uniform_float_values[1].el;
  float x_40 = x_6.x_GLF_uniform_float_values[2].el;
  arr = float[3](x_36, x_38, x_40);
  i = 1;
  {
    while(true) {
      int x_46 = i;
      int x_48 = x_9.x_GLF_uniform_int_values[2].el;
      if ((x_46 < min(x_48, 3))) {
      } else {
        break;
      }
      int x_53 = x_9.x_GLF_uniform_int_values[2].el;
      float x_55 = x_6.x_GLF_uniform_float_values[0].el;
      float x_57 = arr[x_53];
      arr[x_53] = (x_57 + x_55);
      {
        int x_60 = i;
        i = (x_60 + 1);
      }
      continue;
    }
  }
  int x_63 = x_9.x_GLF_uniform_int_values[2].el;
  float x_65 = arr[x_63];
  float x_67 = x_6.x_GLF_uniform_float_values[3].el;
  if ((x_65 == x_67)) {
    int x_73 = x_9.x_GLF_uniform_int_values[1].el;
    int x_76 = x_9.x_GLF_uniform_int_values[0].el;
    int x_79 = x_9.x_GLF_uniform_int_values[0].el;
    int x_82 = x_9.x_GLF_uniform_int_values[1].el;
    float v = float(x_73);
    float v_1 = float(x_76);
    float v_2 = float(x_79);
    x_GLF_color = vec4(v, v_1, v_2, float(x_82));
  } else {
    int x_86 = x_9.x_GLF_uniform_int_values[0].el;
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
