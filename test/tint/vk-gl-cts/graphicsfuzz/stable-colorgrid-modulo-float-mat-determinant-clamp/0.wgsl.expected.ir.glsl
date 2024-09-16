SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct buf0 {
  vec2 resolution;
};

struct main_out {
  vec4 x_GLF_color_1;
};

layout(binding = 0, std140)
uniform tint_symbol_4_1_ubo {
  buf0 tint_symbol_3;
} v;
vec4 tint_symbol = vec4(0.0f);
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_1_loc0_Output;
int tint_mod_i32(int lhs, int rhs) {
  int v_1 = ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs));
  return (lhs - ((lhs / v_1) * v_1));
}
float compute_value_f1_f1_(inout float limit, inout float thirty_two) {
  float result = 0.0f;
  int i = 0;
  result = -0.5f;
  i = 1;
  {
    while(true) {
      int x_111 = i;
      if ((x_111 < 800)) {
      } else {
        break;
      }
      int x_114 = i;
      if ((tint_mod_i32(x_114, 32) == 0)) {
        float x_120 = result;
        result = (x_120 + 0.40000000596046447754f);
      } else {
        int x_122 = i;
        float x_124 = thirty_two;
        float v_2 = float(x_122);
        float v_3 = round(x_124);
        float v_4 = float(x_122);
        if (((v_2 - (v_3 * floor((v_4 / round(x_124))))) <= 0.00999999977648258209f)) {
          float x_130 = result;
          result = (x_130 + 100.0f);
        }
      }
      int x_132 = i;
      float x_134 = limit;
      if ((float(x_132) >= x_134)) {
        float x_138 = result;
        return x_138;
      }
      {
        int x_139 = i;
        i = (x_139 + 1);
      }
      continue;
    }
  }
  float x_141 = result;
  return x_141;
}
void main_1() {
  vec3 c = vec3(0.0f);
  float thirty_two_1 = 0.0f;
  float param = 0.0f;
  float param_1 = 0.0f;
  float param_2 = 0.0f;
  float param_3 = 0.0f;
  int i_1 = 0;
  c = vec3(7.0f, 8.0f, 9.0f);
  float x_56 = v.tint_symbol_3.resolution.x;
  thirty_two_1 = round((x_56 / 8.0f));
  float x_60 = tint_symbol.x;
  param = x_60;
  float x_61 = thirty_two_1;
  param_1 = x_61;
  float x_62 = compute_value_f1_f1_(param, param_1);
  c[0u] = x_62;
  float x_65 = tint_symbol.y;
  param_2 = x_65;
  float x_66 = thirty_two_1;
  param_3 = x_66;
  float x_67 = compute_value_f1_f1_(param_2, param_3);
  c[1u] = x_67;
  float x_70 = c.x;
  float x_72 = c.y;
  c[2u] = (x_70 + x_72);
  i_1 = 0;
  {
    while(true) {
      int x_79 = i_1;
      if ((x_79 < 3)) {
      } else {
        break;
      }
      int x_82 = i_1;
      float x_84 = c[x_82];
      if ((x_84 >= 1.0f)) {
        int x_88 = i_1;
        int x_89 = i_1;
        float x_91 = c[x_89];
        int x_92 = i_1;
        float x_94 = c[x_92];
        c[x_88] = (x_91 * x_94);
      }
      {
        int x_97 = i_1;
        i_1 = (x_97 + 1);
      }
      continue;
    }
  }
  vec3 x_99 = c;
  vec3 x_101 = normalize(abs(x_99));
  x_GLF_color = vec4(x_101[0u], x_101[1u], x_101[2u], 1.0f);
}
main_out tint_symbol_1_inner(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
void main() {
  tint_symbol_1_loc0_Output = tint_symbol_1_inner(gl_FragCoord).x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:22: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:22: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:22: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
