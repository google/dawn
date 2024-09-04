SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[2];
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
  float sums[3] = float[3](0.0f, 0.0f, 0.0f);
  int i = 0;
  mat2x4 indexable = mat2x4(vec4(0.0f), vec4(0.0f));
  float x_40 = x_6.x_GLF_uniform_float_values[0].el;
  float x_42 = x_6.x_GLF_uniform_float_values[0].el;
  float x_44 = x_6.x_GLF_uniform_float_values[0].el;
  sums = float[3](x_40, x_42, x_44);
  i = 0;
  {
    while(true) {
      int x_50 = i;
      int x_52 = x_9.x_GLF_uniform_int_values[0].el;
      int x_54 = x_9.x_GLF_uniform_int_values[0].el;
      if ((x_50 < min(max(x_52, x_54), 2))) {
      } else {
        break;
      }
      int x_59 = x_9.x_GLF_uniform_int_values[2].el;
      float x_61 = x_6.x_GLF_uniform_float_values[0].el;
      int x_65 = i;
      int x_67 = x_9.x_GLF_uniform_int_values[1].el;
      vec4 v = vec4(x_61, 0.0f, 0.0f, 0.0f);
      indexable = mat2x4(v, vec4(0.0f, x_61, 0.0f, 0.0f));
      float x_69 = indexable[x_65][x_67];
      float x_71 = sums[x_59];
      sums[x_59] = (x_71 + x_69);
      {
        int x_74 = i;
        i = (x_74 + 1);
      }
      continue;
    }
  }
  int x_77 = x_9.x_GLF_uniform_int_values[2].el;
  float x_79 = sums[x_77];
  float x_81 = x_6.x_GLF_uniform_float_values[1].el;
  if ((x_79 == x_81)) {
    int x_87 = x_9.x_GLF_uniform_int_values[0].el;
    int x_90 = x_9.x_GLF_uniform_int_values[1].el;
    int x_93 = x_9.x_GLF_uniform_int_values[1].el;
    int x_96 = x_9.x_GLF_uniform_int_values[0].el;
    float v_1 = float(x_87);
    float v_2 = float(x_90);
    float v_3 = float(x_93);
    x_GLF_color = vec4(v_1, v_2, v_3, float(x_96));
  } else {
    int x_100 = x_9.x_GLF_uniform_int_values[1].el;
    float x_101 = float(x_100);
    x_GLF_color = vec4(x_101, x_101, x_101, x_101);
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
