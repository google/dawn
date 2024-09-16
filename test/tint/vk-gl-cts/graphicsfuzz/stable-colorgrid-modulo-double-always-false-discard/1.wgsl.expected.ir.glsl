SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct buf1 {
  vec2 injectionSwitch;
};

struct buf0 {
  vec2 resolution;
};

struct main_out {
  vec4 x_GLF_color_1;
};

layout(binding = 1, std140)
uniform tint_symbol_4_1_ubo {
  buf1 tint_symbol_3;
} v;
layout(binding = 0, std140)
uniform tint_symbol_6_1_ubo {
  buf0 tint_symbol_5;
} v_1;
vec4 tint_symbol = vec4(0.0f);
vec4 x_GLF_color = vec4(0.0f);
bool continue_execution = true;
layout(location = 0) out vec4 tint_symbol_1_loc0_Output;
int tint_mod_i32(int lhs, int rhs) {
  int v_2 = ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs));
  return (lhs - ((lhs / v_2) * v_2));
}
void main_1() {
  vec3 c = vec3(0.0f);
  float x_54 = 0.0f;
  float x_58 = 0.0f;
  float x_59 = 0.0f;
  float x_91 = 0.0f;
  float x_92 = 0.0f;
  float x_135 = 0.0f;
  float x_136 = 0.0f;
  float x_58_phi = 0.0f;
  int x_61_phi = 0;
  float x_91_phi = 0.0f;
  float x_92_phi = 0.0f;
  bool x_93_phi = false;
  float x_95_phi = 0.0f;
  float x_139_phi = 0.0f;
  int x_146_phi = 0;
  c = vec3(7.0f, 8.0f, 9.0f);
  float x_50 = v_1.tint_symbol_5.resolution.x;
  float x_52 = round((x_50 * 0.125f));
  x_54 = tint_symbol.x;
  switch(0u) {
    default:
    {
      x_58_phi = -0.5f;
      x_61_phi = 1;
      {
        while(true) {
          float x_71 = 0.0f;
          float x_79 = 0.0f;
          int x_62 = 0;
          float x_59_phi = 0.0f;
          x_58 = x_58_phi;
          int x_61 = x_61_phi;
          x_91_phi = 0.0f;
          x_92_phi = x_58;
          x_93_phi = false;
          if ((x_61 < 800)) {
          } else {
            break;
          }
          float x_78 = 0.0f;
          float x_79_phi = 0.0f;
          if ((tint_mod_i32(x_61, 32) == 0)) {
            x_71 = (x_58 + 0.40000000596046447754f);
            x_59_phi = x_71;
          } else {
            x_79_phi = x_58;
            float v_3 = float(x_61);
            float v_4 = round(x_52);
            float v_5 = float(x_61);
            if (((v_3 - (v_4 * floor((v_5 / round(x_52))))) <= 0.00999999977648258209f)) {
              x_78 = (x_58 + 100.0f);
              x_79_phi = x_78;
            }
            x_79 = x_79_phi;
            float x_81 = v.tint_symbol_3.injectionSwitch.x;
            float x_83 = v.tint_symbol_3.injectionSwitch.y;
            if ((x_81 > x_83)) {
              continue_execution = false;
            }
            x_59_phi = x_79;
          }
          x_59 = x_59_phi;
          float v_6 = float(x_61);
          if ((v_6 >= x_54)) {
            x_91_phi = x_59;
            x_92_phi = x_59;
            x_93_phi = true;
            break;
          }
          {
            x_62 = (x_61 + 1);
            x_58_phi = x_59;
            x_61_phi = x_62;
          }
          continue;
        }
      }
      x_91 = x_91_phi;
      x_92 = x_92_phi;
      bool x_93 = x_93_phi;
      x_95_phi = x_91;
      if (x_93) {
        break;
      }
      x_95_phi = x_92;
      break;
    }
  }
  float x_98 = 0.0f;
  float x_102 = 0.0f;
  float x_103 = 0.0f;
  float x_102_phi = 0.0f;
  int x_105_phi = 0;
  float x_135_phi = 0.0f;
  float x_136_phi = 0.0f;
  bool x_137_phi = false;
  float x_95 = x_95_phi;
  c[0u] = x_95;
  x_98 = tint_symbol.y;
  switch(0u) {
    default:
    {
      x_102_phi = -0.5f;
      x_105_phi = 1;
      {
        while(true) {
          float x_115 = 0.0f;
          float x_123 = 0.0f;
          int x_106 = 0;
          float x_103_phi = 0.0f;
          x_102 = x_102_phi;
          int x_105 = x_105_phi;
          x_135_phi = 0.0f;
          x_136_phi = x_102;
          x_137_phi = false;
          if ((x_105 < 800)) {
          } else {
            break;
          }
          float x_122 = 0.0f;
          float x_123_phi = 0.0f;
          if ((tint_mod_i32(x_105, 32) == 0)) {
            x_115 = (x_102 + 0.40000000596046447754f);
            x_103_phi = x_115;
          } else {
            x_123_phi = x_102;
            float v_7 = float(x_105);
            float v_8 = round(x_52);
            float v_9 = float(x_105);
            if (((v_7 - (v_8 * floor((v_9 / round(x_52))))) <= 0.00999999977648258209f)) {
              x_122 = (x_102 + 100.0f);
              x_123_phi = x_122;
            }
            x_123 = x_123_phi;
            float x_125 = v.tint_symbol_3.injectionSwitch.x;
            float x_127 = v.tint_symbol_3.injectionSwitch.y;
            if ((x_125 > x_127)) {
              continue_execution = false;
            }
            x_103_phi = x_123;
          }
          x_103 = x_103_phi;
          float v_10 = float(x_105);
          if ((v_10 >= x_98)) {
            x_135_phi = x_103;
            x_136_phi = x_103;
            x_137_phi = true;
            break;
          }
          {
            x_106 = (x_105 + 1);
            x_102_phi = x_103;
            x_105_phi = x_106;
          }
          continue;
        }
      }
      x_135 = x_135_phi;
      x_136 = x_136_phi;
      bool x_137 = x_137_phi;
      x_139_phi = x_135;
      if (x_137) {
        break;
      }
      x_139_phi = x_136;
      break;
    }
  }
  float x_139 = x_139_phi;
  c[1u] = x_139;
  float x_141 = c.x;
  float x_142 = c.y;
  c[2u] = (x_141 + x_142);
  x_146_phi = 0;
  {
    while(true) {
      int x_147 = 0;
      int x_146 = x_146_phi;
      if ((x_146 < 3)) {
      } else {
        break;
      }
      float x_153 = c[x_146];
      if ((x_153 >= 1.0f)) {
        float x_157 = c[x_146];
        float x_158 = c[x_146];
        c[x_146] = (x_157 * x_158);
        float x_161 = v.tint_symbol_3.injectionSwitch.x;
        float x_163 = v.tint_symbol_3.injectionSwitch.y;
        if ((x_161 > x_163)) {
          continue_execution = false;
        }
      }
      {
        x_147 = (x_146 + 1);
        x_146_phi = x_147;
      }
      continue;
    }
  }
  vec3 x_167 = c;
  vec3 x_169 = normalize(abs(x_167));
  x_GLF_color = vec4(x_169[0u], x_169[1u], x_169[2u], 1.0f);
}
main_out tint_symbol_1_inner(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  main_out v_11 = main_out(x_GLF_color);
  if (!(continue_execution)) {
    discard;
  }
  return v_11;
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
