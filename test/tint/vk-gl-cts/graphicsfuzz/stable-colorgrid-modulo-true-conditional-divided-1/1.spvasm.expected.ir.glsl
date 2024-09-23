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
bool continue_execution = true;
layout(location = 0) out vec4 tint_symbol_1_loc0_Output;
int tint_mod_i32(int lhs, int rhs) {
  int v_2 = ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs));
  return (lhs - ((lhs / v_2) * v_2));
}
float compute_value_f1_f1_(inout float limit, inout float thirty_two) {
  float x_104 = 0.0f;
  int x_107 = 0;
  x_104 = -0.5f;
  x_107 = 1;
  {
    while(true) {
      float x_126 = 0.0f;
      float x_125 = 0.0f;
      float x_105 = 0.0f;
      int x_108 = 0;
      if ((x_107 < 800)) {
      } else {
        break;
      }
      float x_124 = 0.0f;
      if ((tint_mod_i32(x_107, 32) == 0)) {
        x_126 = (x_104 + 0.40000000596046447754f);
        x_105 = x_126;
      } else {
        float x_118 = thirty_two;
        x_125 = x_104;
        float v_3 = float(x_107);
        float v_4 = round(x_118);
        float v_5 = float(x_107);
        if (((v_3 - (v_4 * floor((v_5 / round(x_118))))) <= 0.00999999977648258209f)) {
          x_124 = (x_104 + 100.0f);
          x_125 = x_124;
        }
        x_105 = x_125;
      }
      float v_6 = float(x_107);
      if ((v_6 >= limit)) {
        return x_105;
      }
      {
        x_108 = (x_107 + 1);
        x_104 = x_105;
        x_107 = x_108;
      }
      continue;
    }
  }
  return x_104;
}
void main_1() {
  vec3 c = vec3(0.0f);
  float param = 0.0f;
  float param_1 = 0.0f;
  float param_2 = 0.0f;
  float param_3 = 0.0f;
  vec3 x_54 = vec3(0.0f);
  int x_74 = 0;
  c = vec3(7.0f, 8.0f, 9.0f);
  float x_58 = round((v.tint_symbol_3.resolution.x * 0.125f));
  param = tint_symbol.x;
  param_1 = x_58;
  float x_61 = compute_value_f1_f1_(param, param_1);
  c[0u] = x_61;
  param_2 = tint_symbol.y;
  param_3 = x_58;
  float x_65 = compute_value_f1_f1_(param_2, param_3);
  c[1u] = x_65;
  float x_67 = c.x;
  x_54 = c;
  c[2u] = (x_67 + x_54.y);
  x_74 = 0;
  {
    while(true) {
      int x_75 = 0;
      if ((x_74 < 3)) {
      } else {
        break;
      }
      if ((c[x_74] >= 1.0f)) {
        if ((v_1.tint_symbol_5.injectionSwitch.x > v_1.tint_symbol_5.injectionSwitch.y)) {
          continue_execution = false;
        }
        c[x_74] = (c[x_74] * c[x_74]);
      }
      {
        x_75 = (x_74 + 1);
        x_74 = x_75;
      }
      continue;
    }
  }
  vec3 x_97 = normalize(abs(c));
  x_GLF_color = vec4(x_97[0u], x_97[1u], x_97[2u], 1.0f);
}
main_out tint_symbol_1_inner(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  main_out v_7 = main_out(x_GLF_color);
  if (!(continue_execution)) {
    discard;
  }
  return v_7;
}
void main() {
  tint_symbol_1_loc0_Output = tint_symbol_1_inner(gl_FragCoord).x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:31: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:31: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:31: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
