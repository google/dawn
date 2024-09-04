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
  if ((x_5.x_GLF_uniform_int_values[1].el == 1)) {
    vec2 v = vec2(f, 0.0f);
    m = mat2(v, vec2(0.0f, f));
  }
  i = x_5.x_GLF_uniform_int_values[1].el;
  {
    while(true) {
      if ((i < x_5.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      j = x_5.x_GLF_uniform_int_values[1].el;
      {
        while(true) {
          if ((j < x_5.x_GLF_uniform_int_values[0].el)) {
          } else {
            break;
          }
          int x_66 = i;
          int x_67 = j;
          m[x_66][x_67] = float(((i * x_5.x_GLF_uniform_int_values[0].el) + j));
          {
            j = (j + 1);
          }
          continue;
        }
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  float v_1 = float(x_5.x_GLF_uniform_int_values[1].el);
  vec2 v_2 = vec2(v_1, float(x_5.x_GLF_uniform_int_values[2].el));
  float v_3 = float(x_5.x_GLF_uniform_int_values[0].el);
  mat2 x_95 = mat2(v_2, vec2(v_3, float(x_5.x_GLF_uniform_int_values[3].el)));
  bool v_4 = all((m[0u] == x_95[0u]));
  if ((v_4 & all((m[1u] == x_95[1u])))) {
    float v_5 = float(x_5.x_GLF_uniform_int_values[2].el);
    float v_6 = float(x_5.x_GLF_uniform_int_values[1].el);
    float v_7 = float(x_5.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v_5, v_6, v_7, float(x_5.x_GLF_uniform_int_values[2].el));
  } else {
    x_GLF_color = vec4(float(x_5.x_GLF_uniform_int_values[1].el));
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
