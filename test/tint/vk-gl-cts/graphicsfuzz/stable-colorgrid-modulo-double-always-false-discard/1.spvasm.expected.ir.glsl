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
  int x_61 = 0;
  float x_59 = 0.0f;
  float x_91 = 0.0f;
  float x_92 = 0.0f;
  bool x_93 = false;
  float x_95 = 0.0f;
  float x_135 = 0.0f;
  float x_136 = 0.0f;
  float x_139 = 0.0f;
  int x_146 = 0;
  c = vec3(7.0f, 8.0f, 9.0f);
  float x_52 = round((v_1.tint_symbol_5.resolution.x * 0.125f));
  x_54 = tint_symbol.x;
  switch(0u) {
    default:
    {
      x_58 = -0.5f;
      x_61 = 1;
      {
        while(true) {
          float x_71 = 0.0f;
          float x_79 = 0.0f;
          int x_62 = 0;
          x_91 = 0.0f;
          x_92 = x_58;
          x_93 = false;
          if ((x_61 < 800)) {
          } else {
            break;
          }
          float x_78 = 0.0f;
          if ((tint_mod_i32(x_61, 32) == 0)) {
            x_71 = (x_58 + 0.40000000596046447754f);
            x_59 = x_71;
          } else {
            x_79 = x_58;
            float v_3 = float(x_61);
            float v_4 = round(x_52);
            float v_5 = float(x_61);
            if (((v_3 - (v_4 * floor((v_5 / round(x_52))))) <= 0.00999999977648258209f)) {
              x_78 = (x_58 + 100.0f);
              x_79 = x_78;
            }
            if ((v.tint_symbol_3.injectionSwitch.x > v.tint_symbol_3.injectionSwitch.y)) {
              continue_execution = false;
            }
            x_59 = x_79;
          }
          float v_6 = float(x_61);
          if ((v_6 >= x_54)) {
            x_91 = x_59;
            x_92 = x_59;
            x_93 = true;
            break;
          }
          {
            x_62 = (x_61 + 1);
            x_58 = x_59;
            x_61 = x_62;
          }
          continue;
        }
      }
      x_95 = x_91;
      if (x_93) {
        break;
      }
      x_95 = x_92;
      break;
    }
  }
  float x_98 = 0.0f;
  float x_102 = 0.0f;
  int x_105 = 0;
  float x_103 = 0.0f;
  bool x_137 = false;
  c[0u] = x_95;
  x_98 = tint_symbol.y;
  switch(0u) {
    default:
    {
      x_102 = -0.5f;
      x_105 = 1;
      {
        while(true) {
          float x_115 = 0.0f;
          float x_123 = 0.0f;
          int x_106 = 0;
          x_135 = 0.0f;
          x_136 = x_102;
          x_137 = false;
          if ((x_105 < 800)) {
          } else {
            break;
          }
          float x_122 = 0.0f;
          if ((tint_mod_i32(x_105, 32) == 0)) {
            x_115 = (x_102 + 0.40000000596046447754f);
            x_103 = x_115;
          } else {
            x_123 = x_102;
            float v_7 = float(x_105);
            float v_8 = round(x_52);
            float v_9 = float(x_105);
            if (((v_7 - (v_8 * floor((v_9 / round(x_52))))) <= 0.00999999977648258209f)) {
              x_122 = (x_102 + 100.0f);
              x_123 = x_122;
            }
            if ((v.tint_symbol_3.injectionSwitch.x > v.tint_symbol_3.injectionSwitch.y)) {
              continue_execution = false;
            }
            x_103 = x_123;
          }
          float v_10 = float(x_105);
          if ((v_10 >= x_98)) {
            x_135 = x_103;
            x_136 = x_103;
            x_137 = true;
            break;
          }
          {
            x_106 = (x_105 + 1);
            x_102 = x_103;
            x_105 = x_106;
          }
          continue;
        }
      }
      x_139 = x_135;
      if (x_137) {
        break;
      }
      x_139 = x_136;
      break;
    }
  }
  c[1u] = x_139;
  c[2u] = (c.x + c.y);
  x_146 = 0;
  {
    while(true) {
      int x_147 = 0;
      if ((x_146 < 3)) {
      } else {
        break;
      }
      if ((c[x_146] >= 1.0f)) {
        c[x_146] = (c[x_146] * c[x_146]);
        if ((v.tint_symbol_3.injectionSwitch.x > v.tint_symbol_3.injectionSwitch.y)) {
          continue_execution = false;
        }
      }
      {
        x_147 = (x_146 + 1);
        x_146 = x_147;
      }
      continue;
    }
  }
  vec3 x_169 = normalize(abs(c));
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
