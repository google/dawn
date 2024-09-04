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
  sums = float[3](x_6.x_GLF_uniform_float_values[0].el, x_6.x_GLF_uniform_float_values[0].el, x_6.x_GLF_uniform_float_values[0].el);
  i = 0;
  {
    while(true) {
      int v = i;
      if ((v < min(max(x_9.x_GLF_uniform_int_values[0].el, x_9.x_GLF_uniform_int_values[0].el), 2))) {
      } else {
        break;
      }
      int x_59 = x_9.x_GLF_uniform_int_values[2].el;
      float x_61 = x_6.x_GLF_uniform_float_values[0].el;
      int x_65 = i;
      int x_67 = x_9.x_GLF_uniform_int_values[1].el;
      vec4 v_1 = vec4(x_61, 0.0f, 0.0f, 0.0f);
      indexable = mat2x4(v_1, vec4(0.0f, x_61, 0.0f, 0.0f));
      sums[x_59] = (sums[x_59] + indexable[x_65][x_67]);
      {
        i = (i + 1);
      }
      continue;
    }
  }
  if ((sums[x_9.x_GLF_uniform_int_values[2].el] == x_6.x_GLF_uniform_float_values[1].el)) {
    float v_2 = float(x_9.x_GLF_uniform_int_values[0].el);
    float v_3 = float(x_9.x_GLF_uniform_int_values[1].el);
    float v_4 = float(x_9.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v_2, v_3, v_4, float(x_9.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(float(x_9.x_GLF_uniform_int_values[1].el));
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
