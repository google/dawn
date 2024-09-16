SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct buf0 {
  vec2 resolution;
};

struct buf1 {
  vec2 injectionSwitch;
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
layout(binding = 1, std140)
uniform tint_symbol_6_1_ubo {
  buf1 tint_symbol_5;
} v_1;
layout(location = 0) out vec4 tint_symbol_1_loc0_Output;
int tint_mod_i32(int lhs, int rhs) {
  int v_2 = ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs));
  return (lhs - ((lhs / v_2) * v_2));
}
float compute_value_f1_f1_(inout float limit, inout float thirty_two) {
  float result = 0.0f;
  int i = 0;
  result = -0.5f;
  i = 1;
  {
    while(true) {
      int x_125 = i;
      if ((x_125 < 800)) {
      } else {
        break;
      }
      int x_128 = i;
      if ((tint_mod_i32(x_128, 32) == 0)) {
        float x_134 = result;
        result = (x_134 + 0.40000000596046447754f);
      } else {
        int x_136 = i;
        float x_138 = thirty_two;
        float v_3 = float(x_136);
        float v_4 = round(x_138);
        float v_5 = float(x_136);
        if (((v_3 - (v_4 * floor((v_5 / round(x_138))))) <= 0.00999999977648258209f)) {
          float x_144 = result;
          result = (x_144 + 100.0f);
        }
      }
      int x_146 = i;
      float x_148 = limit;
      if ((float(x_146) >= x_148)) {
        float x_152 = result;
        return x_152;
      }
      {
        int x_153 = i;
        i = (x_153 + 1);
      }
      continue;
    }
  }
  float x_155 = result;
  return x_155;
}
void main_1() {
  vec3 c = vec3(0.0f);
  float thirty_two_1 = 0.0f;
  float param = 0.0f;
  float param_1 = 0.0f;
  float param_2 = 0.0f;
  float param_3 = 0.0f;
  int i_1 = 0;
  vec3 x_58 = vec3(0.0f);
  c = vec3(7.0f, 8.0f, 9.0f);
  float x_60 = v.tint_symbol_3.resolution.x;
  thirty_two_1 = round((x_60 / 8.0f));
  float x_64 = tint_symbol.x;
  param = x_64;
  float x_65 = thirty_two_1;
  param_1 = x_65;
  float x_66 = compute_value_f1_f1_(param, param_1);
  c[0u] = x_66;
  float x_69 = tint_symbol.y;
  param_2 = x_69;
  float x_70 = thirty_two_1;
  param_3 = x_70;
  float x_71 = compute_value_f1_f1_(param_2, param_3);
  c[1u] = x_71;
  float x_74 = c.x;
  float x_76 = c.y;
  c[2u] = (x_74 + x_76);
  i_1 = 0;
  {
    while(true) {
      int x_83 = i_1;
      if ((x_83 < 3)) {
      } else {
        break;
      }
      int x_86 = i_1;
      float x_88 = c[x_86];
      if ((x_88 >= 1.0f)) {
        int x_92 = i_1;
        int x_93 = i_1;
        float x_95 = c[x_93];
        int x_96 = i_1;
        float x_98 = c[x_96];
        c[x_92] = (x_95 * x_98);
      }
      {
        int x_101 = i_1;
        i_1 = (x_101 + 1);
      }
      continue;
    }
  }
  float x_104 = v_1.tint_symbol_5.injectionSwitch.x;
  float x_106 = v_1.tint_symbol_5.injectionSwitch.y;
  if ((x_104 < x_106)) {
    vec3 x_111 = c;
    x_58 = abs(x_111);
  } else {
    vec3 x_113 = c;
    x_58 = x_113;
  }
  vec3 x_114 = x_58;
  vec3 x_115 = normalize(x_114);
  x_GLF_color = vec4(x_115[0u], x_115[1u], x_115[2u], 1.0f);
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
ERROR: 0:30: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:30: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:30: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
