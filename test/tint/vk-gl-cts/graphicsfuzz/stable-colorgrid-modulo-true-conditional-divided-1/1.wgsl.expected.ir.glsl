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
  float x_104_phi = 0.0f;
  int x_107_phi = 0;
  x_104_phi = -0.5f;
  x_107_phi = 1;
  {
    while(true) {
      float x_126 = 0.0f;
      float x_125 = 0.0f;
      int x_108 = 0;
      float x_105_phi = 0.0f;
      x_104 = x_104_phi;
      int x_107 = x_107_phi;
      if ((x_107 < 800)) {
      } else {
        break;
      }
      float x_124 = 0.0f;
      float x_125_phi = 0.0f;
      if ((tint_mod_i32(x_107, 32) == 0)) {
        x_126 = (x_104 + 0.40000000596046447754f);
        x_105_phi = x_126;
      } else {
        float x_118 = thirty_two;
        x_125_phi = x_104;
        float v_3 = float(x_107);
        float v_4 = round(x_118);
        float v_5 = float(x_107);
        if (((v_3 - (v_4 * floor((v_5 / round(x_118))))) <= 0.00999999977648258209f)) {
          x_124 = (x_104 + 100.0f);
          x_125_phi = x_124;
        }
        x_125 = x_125_phi;
        x_105_phi = x_125;
      }
      float x_105 = 0.0f;
      x_105 = x_105_phi;
      float x_128 = limit;
      if ((float(x_107) >= x_128)) {
        return x_105;
      }
      {
        x_108 = (x_107 + 1);
        x_104_phi = x_105;
        x_107_phi = x_108;
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
  int x_74_phi = 0;
  c = vec3(7.0f, 8.0f, 9.0f);
  float x_56 = v.tint_symbol_3.resolution.x;
  float x_58 = round((x_56 * 0.125f));
  float x_60 = tint_symbol.x;
  param = x_60;
  param_1 = x_58;
  float x_61 = compute_value_f1_f1_(param, param_1);
  c[0u] = x_61;
  float x_64 = tint_symbol.y;
  param_2 = x_64;
  param_3 = x_58;
  float x_65 = compute_value_f1_f1_(param_2, param_3);
  c[1u] = x_65;
  float x_67 = c.x;
  vec3 x_68 = c;
  x_54 = x_68;
  float x_70 = x_54.y;
  c[2u] = (x_67 + x_70);
  x_74_phi = 0;
  {
    while(true) {
      int x_75 = 0;
      int x_74 = x_74_phi;
      if ((x_74 < 3)) {
      } else {
        break;
      }
      float x_81 = c[x_74];
      if ((x_81 >= 1.0f)) {
        float x_86 = v_1.tint_symbol_5.injectionSwitch.x;
        float x_88 = v_1.tint_symbol_5.injectionSwitch.y;
        if ((x_86 > x_88)) {
          continue_execution = false;
        }
        float x_92 = c[x_74];
        float x_93 = c[x_74];
        c[x_74] = (x_92 * x_93);
      }
      {
        x_75 = (x_74 + 1);
        x_74_phi = x_75;
      }
      continue;
    }
  }
  vec3 x_95 = c;
  vec3 x_97 = normalize(abs(x_95));
  x_GLF_color = vec4(x_97[0u], x_97[1u], x_97[2u], 1.0f);
}
main_out tint_symbol_1_inner(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  main_out v_6 = main_out(x_GLF_color);
  if (!(continue_execution)) {
    discard;
  }
  return v_6;
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
