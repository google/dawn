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
      int x_136 = i;
      if ((x_136 < 800)) {
      } else {
        break;
      }
      int x_139 = i;
      if ((tint_mod_i32(x_139, 32) == 0)) {
        float x_145 = result;
        result = (x_145 + 0.40000000596046447754f);
      } else {
        int x_147 = i;
        float x_149 = thirty_two;
        float v_2 = float(x_147);
        float v_3 = round(x_149);
        float v_4 = float(x_147);
        if (((v_2 - (v_3 * floor((v_4 / round(x_149))))) <= 0.00999999977648258209f)) {
          float x_155 = result;
          result = (x_155 + 100.0f);
        }
      }
      int x_157 = i;
      float x_159 = limit;
      if ((float(x_157) >= x_159)) {
        float x_163 = result;
        return x_163;
      }
      {
        int x_164 = i;
        i = (x_164 + 1);
      }
      continue;
    }
  }
  float x_166 = result;
  return x_166;
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
  float x_63 = v.tint_symbol_3.resolution.x;
  thirty_two_1 = round((x_63 / 8.0f));
  float x_67 = tint_symbol.x;
  param = x_67;
  float x_68 = thirty_two_1;
  param_1 = x_68;
  float x_69 = compute_value_f1_f1_(param, param_1);
  c[0u] = x_69;
  float x_72 = tint_symbol.y;
  param_2 = x_72;
  float x_73 = thirty_two_1;
  param_3 = x_73;
  float x_74 = compute_value_f1_f1_(param_2, param_3);
  c[1u] = x_74;
  vec3 x_76 = c;
  vec3 x_79 = c;
  vec2 v_5 = vec2(x_79[0u], x_79[1u]);
  mat4x2 x_87 = mat4x2(v_5, vec2(x_79[2u], 1.0f), vec2(1.0f, 0.0f), vec2(1.0f, 0.0f));
  c[2u] = ((x_76 * mat3(vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)))[0u] + vec3(x_87[0u][0u], x_87[0u][1u], x_87[1u][0u])[1u]);
  i_1 = 0;
  {
    while(true) {
      int x_99 = i_1;
      if ((x_99 < 3)) {
      } else {
        break;
      }
      int x_102 = i_1;
      float x_104 = c[x_102];
      if ((x_104 >= 1.0f)) {
        int x_108 = i_1;
        int x_109 = i_1;
        float x_111 = c[x_109];
        int x_112 = i_1;
        float x_114 = c[x_112];
        c[x_108] = (x_111 * x_114);
        float x_118 = tint_symbol.y;
        if ((x_118 < 0.0f)) {
          break;
        }
      }
      {
        int x_122 = i_1;
        i_1 = (x_122 + 1);
      }
      continue;
    }
  }
  vec3 x_124 = c;
  vec3 x_126 = normalize(abs(x_124));
  x_GLF_color = vec4(x_126[0u], x_126[1u], x_126[2u], 1.0f);
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
