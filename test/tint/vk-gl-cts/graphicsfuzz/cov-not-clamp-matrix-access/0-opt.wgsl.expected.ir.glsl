SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[5];
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
  mat2 M1 = mat2(vec2(0.0f), vec2(0.0f));
  float a = 0.0f;
  int c = 0;
  float x_41 = x_6.x_GLF_uniform_float_values[1].el;
  float x_43 = x_6.x_GLF_uniform_float_values[2].el;
  float x_45 = x_6.x_GLF_uniform_float_values[3].el;
  float x_47 = x_6.x_GLF_uniform_float_values[4].el;
  vec2 v = vec2(x_41, x_43);
  M1 = mat2(v, vec2(x_45, x_47));
  float x_52 = x_6.x_GLF_uniform_float_values[1].el;
  a = x_52;
  int x_54 = x_10.x_GLF_uniform_int_values[1].el;
  c = x_54;
  {
    while(true) {
      int x_59 = c;
      int x_61 = x_10.x_GLF_uniform_int_values[0].el;
      if ((x_59 < x_61)) {
      } else {
        break;
      }
      int x_65 = x_10.x_GLF_uniform_int_values[2].el;
      int x_66 = c;
      float x_70 = M1[x_65][min(max(~(x_66), 0), 1)];
      float x_71 = a;
      a = (x_71 + x_70);
      {
        int x_73 = c;
        c = (x_73 + 1);
      }
      continue;
    }
  }
  float x_75 = a;
  float x_77 = x_6.x_GLF_uniform_float_values[0].el;
  if ((x_75 == x_77)) {
    int x_83 = x_10.x_GLF_uniform_int_values[2].el;
    int x_86 = x_10.x_GLF_uniform_int_values[1].el;
    int x_89 = x_10.x_GLF_uniform_int_values[1].el;
    int x_92 = x_10.x_GLF_uniform_int_values[2].el;
    float v_1 = float(x_83);
    float v_2 = float(x_86);
    float v_3 = float(x_89);
    x_GLF_color = vec4(v_1, v_2, v_3, float(x_92));
  } else {
    int x_96 = x_10.x_GLF_uniform_int_values[2].el;
    float x_97 = float(x_96);
    x_GLF_color = vec4(x_97, x_97, x_97, x_97);
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
