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
  strided_arr_1 x_GLF_uniform_int_values[4];
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
  vec4 v0 = vec4(0.0f);
  vec4 v1 = vec4(0.0f);
  int a = 0;
  int c = 0;
  float x_41 = x_6.x_GLF_uniform_float_values[1].el;
  v0 = vec4(x_41, x_41, x_41, x_41);
  float x_44 = x_6.x_GLF_uniform_float_values[0].el;
  v1 = vec4(x_44, x_44, x_44, x_44);
  int x_47 = x_10.x_GLF_uniform_int_values[1].el;
  a = x_47;
  {
    while(true) {
      int x_52 = a;
      int x_54 = x_10.x_GLF_uniform_int_values[0].el;
      if ((x_52 < x_54)) {
      } else {
        break;
      }
      int x_58 = x_10.x_GLF_uniform_int_values[3].el;
      c = x_58;
      {
        while(true) {
          int x_63 = c;
          int x_65 = x_10.x_GLF_uniform_int_values[2].el;
          if ((x_63 < x_65)) {
          } else {
            break;
          }
          int x_68 = c;
          int x_69 = min(max(x_68, 0), 3);
          float x_71 = x_6.x_GLF_uniform_float_values[1].el;
          float x_73 = v0[x_69];
          v0[x_69] = (x_73 - x_71);
          int x_77 = x_10.x_GLF_uniform_int_values[1].el;
          int x_79 = x_10.x_GLF_uniform_int_values[3].el;
          if ((x_77 == x_79)) {
            int x_83 = a;
            float x_85 = x_6.x_GLF_uniform_float_values[1].el;
            float x_87 = x_6.x_GLF_uniform_float_values[1].el;
            float x_89 = x_6.x_GLF_uniform_float_values[1].el;
            vec4 x_91 = v0;
            int x_93 = a;
            v1[x_83] = smoothstep(vec4(x_85, x_87, x_89, 3.0f), vec4(1.0f), x_91)[x_93];
          }
          {
            int x_96 = c;
            c = (x_96 + 1);
          }
          continue;
        }
      }
      {
        int x_98 = a;
        a = (x_98 + 1);
      }
      continue;
    }
  }
  float x_101 = v1.x;
  float x_103 = x_6.x_GLF_uniform_float_values[0].el;
  if ((x_101 == x_103)) {
    int x_109 = x_10.x_GLF_uniform_int_values[1].el;
    int x_112 = x_10.x_GLF_uniform_int_values[3].el;
    int x_115 = x_10.x_GLF_uniform_int_values[3].el;
    int x_118 = x_10.x_GLF_uniform_int_values[1].el;
    float v = float(x_109);
    float v_1 = float(x_112);
    float v_2 = float(x_115);
    x_GLF_color = vec4(v, v_1, v_2, float(x_118));
  } else {
    int x_122 = x_10.x_GLF_uniform_int_values[3].el;
    float x_123 = float(x_122);
    x_GLF_color = vec4(x_123, x_123, x_123, x_123);
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
