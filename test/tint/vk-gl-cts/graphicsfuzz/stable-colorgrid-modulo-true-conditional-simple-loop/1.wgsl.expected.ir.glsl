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
layout(binding = 1, std140)
uniform tint_symbol_6_1_ubo {
  buf1 tint_symbol_5;
} v_1;
vec4 x_GLF_color = vec4(0.0f);
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
      int x_144 = i;
      if ((x_144 < 800)) {
      } else {
        break;
      }
      int x_147 = i;
      if ((tint_mod_i32(x_147, 32) == 0)) {
        float x_153 = result;
        result = (x_153 + 0.40000000596046447754f);
      } else {
        int x_155 = i;
        float x_157 = thirty_two;
        float v_3 = float(x_155);
        float v_4 = round(x_157);
        float v_5 = float(x_155);
        if (((v_3 - (v_4 * floor((v_5 / round(x_157))))) <= 0.00999999977648258209f)) {
          float x_163 = result;
          result = (x_163 + 100.0f);
        }
      }
      int x_165 = i;
      float x_167 = limit;
      if ((float(x_165) >= x_167)) {
        float x_171 = result;
        return x_171;
      }
      {
        int x_172 = i;
        i = (x_172 + 1);
      }
      continue;
    }
  }
  float x_174 = result;
  return x_174;
}
void main_1() {
  vec3 c = vec3(0.0f);
  float thirty_two_1 = 0.0f;
  float param = 0.0f;
  float param_1 = 0.0f;
  float param_2 = 0.0f;
  float param_3 = 0.0f;
  vec3 x_61 = vec3(0.0f);
  int i_1 = 0;
  float j = 0.0f;
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
  float x_77 = c.x;
  if (true) {
    vec3 x_81 = c;
    x_61 = x_81;
  } else {
    vec3 x_82 = c;
    float x_84 = v_1.tint_symbol_5.injectionSwitch.x;
    x_61 = (x_82 * x_84);
  }
  float x_87 = x_61.y;
  c[2u] = (x_77 + x_87);
  i_1 = 0;
  {
    while(true) {
      int x_94 = i_1;
      if ((x_94 < 3)) {
      } else {
        break;
      }
      int x_97 = i_1;
      float x_99 = c[x_97];
      if ((x_99 >= 1.0f)) {
        int x_103 = i_1;
        int x_104 = i_1;
        float x_106 = c[x_104];
        int x_107 = i_1;
        float x_109 = c[x_107];
        c[x_103] = (x_106 * x_109);
      }
      j = 0.0f;
      {
        while(true) {
          float x_117 = v_1.tint_symbol_5.injectionSwitch.x;
          float x_119 = v_1.tint_symbol_5.injectionSwitch.y;
          if ((x_117 > x_119)) {
          } else {
            break;
          }
          float x_122 = j;
          float x_124 = v_1.tint_symbol_5.injectionSwitch.x;
          if ((x_122 >= x_124)) {
            break;
          }
          float x_128 = j;
          j = (x_128 + 1.0f);
          {
          }
          continue;
        }
      }
      {
        int x_130 = i_1;
        i_1 = (x_130 + 1);
      }
      continue;
    }
  }
  vec3 x_132 = c;
  vec3 x_134 = normalize(abs(x_132));
  x_GLF_color = vec4(x_134[0u], x_134[1u], x_134[2u], 1.0f);
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
