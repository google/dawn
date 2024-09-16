SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[4];
};

struct main_out {
  vec4 x_GLF_color_1;
};

layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  buf0 tint_symbol_1;
} v;
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_loc0_Output;
void main_1() {
  mat2 m = mat2(vec2(0.0f), vec2(0.0f));
  float f = 0.0f;
  int i = 0;
  int j = 0;
  if ((v.tint_symbol_1.x_GLF_uniform_int_values[1].el == 1)) {
    vec2 v_1 = vec2(f, 0.0f);
    m = mat2(v_1, vec2(0.0f, f));
  }
  i = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
  {
    while(true) {
      if ((i < v.tint_symbol_1.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      j = v.tint_symbol_1.x_GLF_uniform_int_values[1].el;
      {
        while(true) {
          if ((j < v.tint_symbol_1.x_GLF_uniform_int_values[0].el)) {
          } else {
            break;
          }
          int x_66 = i;
          int x_67 = j;
          m[x_66][x_67] = float(((i * v.tint_symbol_1.x_GLF_uniform_int_values[0].el) + j));
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
  float v_2 = float(v.tint_symbol_1.x_GLF_uniform_int_values[1].el);
  vec2 v_3 = vec2(v_2, float(v.tint_symbol_1.x_GLF_uniform_int_values[2].el));
  float v_4 = float(v.tint_symbol_1.x_GLF_uniform_int_values[0].el);
  mat2 x_95 = mat2(v_3, vec2(v_4, float(v.tint_symbol_1.x_GLF_uniform_int_values[3].el)));
  bool v_5 = all((m[0u] == x_95[0u]));
  if ((v_5 & all((m[1u] == x_95[1u])))) {
    float v_6 = float(v.tint_symbol_1.x_GLF_uniform_int_values[2].el);
    float v_7 = float(v.tint_symbol_1.x_GLF_uniform_int_values[1].el);
    float v_8 = float(v.tint_symbol_1.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v_6, v_7, v_8, float(v.tint_symbol_1.x_GLF_uniform_int_values[2].el));
  } else {
    x_GLF_color = vec4(float(v.tint_symbol_1.x_GLF_uniform_int_values[1].el));
  }
}
main_out tint_symbol_inner() {
  main_1();
  return main_out(x_GLF_color);
}
void main() {
  tint_symbol_loc0_Output = tint_symbol_inner().x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:66: 'all' : no matching overloaded function found 
ERROR: 0:66: '=' :  cannot convert from ' const float' to ' temp bool'
ERROR: 0:66: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
