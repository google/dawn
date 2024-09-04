SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[4];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_5;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  mat2 m = mat2(vec2(0.0f), vec2(0.0f));
  float f = 0.0f;
  int i = 0;
  int j = 0;
  int x_36 = x_5.x_GLF_uniform_int_values[1].el;
  if ((x_36 == 1)) {
    float x_40 = f;
    vec2 v = vec2(x_40, 0.0f);
    m = mat2(v, vec2(0.0f, x_40));
  }
  int x_45 = x_5.x_GLF_uniform_int_values[1].el;
  i = x_45;
  {
    while(true) {
      int x_50 = i;
      int x_52 = x_5.x_GLF_uniform_int_values[0].el;
      if ((x_50 < x_52)) {
      } else {
        break;
      }
      int x_56 = x_5.x_GLF_uniform_int_values[1].el;
      j = x_56;
      {
        while(true) {
          int x_61 = j;
          int x_63 = x_5.x_GLF_uniform_int_values[0].el;
          if ((x_61 < x_63)) {
          } else {
            break;
          }
          int x_66 = i;
          int x_67 = j;
          int x_68 = i;
          int x_70 = x_5.x_GLF_uniform_int_values[0].el;
          int x_72 = j;
          m[x_66][x_67] = float(((x_68 * x_70) + x_72));
          {
            int x_76 = j;
            j = (x_76 + 1);
          }
          continue;
        }
      }
      {
        int x_78 = i;
        i = (x_78 + 1);
      }
      continue;
    }
  }
  mat2 x_80 = m;
  int x_82 = x_5.x_GLF_uniform_int_values[1].el;
  int x_85 = x_5.x_GLF_uniform_int_values[2].el;
  int x_88 = x_5.x_GLF_uniform_int_values[0].el;
  int x_91 = x_5.x_GLF_uniform_int_values[3].el;
  float v_1 = float(x_82);
  vec2 v_2 = vec2(v_1, float(x_85));
  float v_3 = float(x_88);
  mat2 x_95 = mat2(v_2, vec2(v_3, float(x_91)));
  bool v_4 = all((x_80[0u] == x_95[0u]));
  if ((v_4 & all((x_80[1u] == x_95[1u])))) {
    int x_109 = x_5.x_GLF_uniform_int_values[2].el;
    int x_112 = x_5.x_GLF_uniform_int_values[1].el;
    int x_115 = x_5.x_GLF_uniform_int_values[1].el;
    int x_118 = x_5.x_GLF_uniform_int_values[2].el;
    float v_5 = float(x_109);
    float v_6 = float(x_112);
    float v_7 = float(x_115);
    x_GLF_color = vec4(v_5, v_6, v_7, float(x_118));
  } else {
    int x_122 = x_5.x_GLF_uniform_int_values[1].el;
    float x_123 = float(x_122);
    x_GLF_color = vec4(x_123, x_123, x_123, x_123);
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
