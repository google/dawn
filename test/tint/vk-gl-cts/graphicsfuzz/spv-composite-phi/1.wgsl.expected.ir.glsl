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
void main_1() {
  vec3 c = vec3(0.0f);
  float x_53 = 0.0f;
  float x_57 = 0.0f;
  float x_58 = 0.0f;
  float x_83 = 0.0f;
  float x_84 = 0.0f;
  float x_124 = 0.0f;
  float x_125 = 0.0f;
  float x_57_phi = 0.0f;
  int x_60_phi = 0;
  float x_83_phi = 0.0f;
  float x_84_phi = 0.0f;
  bool x_85_phi = false;
  float x_87_phi = 0.0f;
  float x_128_phi = 0.0f;
  int x_135_phi = 0;
  c = vec3(7.0f, 8.0f, 9.0f);
  float x_47 = v.tint_symbol_3.resolution.x;
  vec2 x_48 = vec2(1.0f, x_47);
  float x_50 = round((x_47 * 0.125f));
  vec2 x_51 = vec2(0.0f, -0.5f);
  x_53 = tint_symbol.x;
  switch(0u) {
    default:
    {
      x_57_phi = -0.5f;
      x_60_phi = 1;
      {
        while(true) {
          float x_70 = 0.0f;
          float x_78 = 0.0f;
          int x_61 = 0;
          float x_58_phi = 0.0f;
          x_57 = x_57_phi;
          int x_60 = x_60_phi;
          x_83_phi = 0.0f;
          x_84_phi = x_57;
          x_85_phi = false;
          if ((x_60 < 800)) {
          } else {
            break;
          }
          float x_77 = 0.0f;
          float x_78_phi = 0.0f;
          if ((tint_mod_i32(x_60, 32) == 0)) {
            x_70 = (x_57 + 0.40000000596046447754f);
            x_58_phi = x_70;
          } else {
            x_78_phi = x_57;
            float v_2 = float(x_60);
            float v_3 = round(x_50);
            float v_4 = float(x_60);
            if (((v_2 - (v_3 * floor((v_4 / round(x_50))))) <= 0.00999999977648258209f)) {
              x_77 = (x_57 + 100.0f);
              x_78_phi = x_77;
            }
            x_78 = x_78_phi;
            x_58_phi = x_78;
          }
          x_58 = x_58_phi;
          float v_5 = float(x_60);
          if ((v_5 >= x_53)) {
            x_83_phi = x_58;
            x_84_phi = x_58;
            x_85_phi = true;
            break;
          }
          {
            x_61 = (x_60 + 1);
            x_57_phi = x_58;
            x_60_phi = x_61;
          }
          continue;
        }
      }
      x_83 = x_83_phi;
      x_84 = x_84_phi;
      bool x_85 = x_85_phi;
      x_87_phi = x_83;
      if (x_85) {
        break;
      }
      x_87_phi = x_84;
      break;
    }
  }
  float x_92 = 0.0f;
  float x_98 = 0.0f;
  float x_99 = 0.0f;
  float x_98_phi = 0.0f;
  int x_101_phi = 0;
  float x_124_phi = 0.0f;
  float x_125_phi = 0.0f;
  bool x_126_phi = false;
  float x_87 = x_87_phi;
  vec4 x_89 = vec4(x_84, 0.40000000596046447754f, x_83, 0.40000000596046447754f);
  c[0u] = x_87;
  x_92 = tint_symbol.y;
  switch(0u) {
    default:
    {
      vec4 x_95 = vec4(x_51, 0.0f, x_57);
      float x_96 = vec3(x_48, -0.5f)[2u];
      x_98_phi = x_96;
      x_101_phi = 1;
      {
        while(true) {
          float x_111 = 0.0f;
          float x_119 = 0.0f;
          int x_102 = 0;
          float x_99_phi = 0.0f;
          x_98 = x_98_phi;
          int x_101 = x_101_phi;
          x_124_phi = 0.0f;
          x_125_phi = x_98;
          x_126_phi = false;
          if ((x_101 < 800)) {
          } else {
            break;
          }
          float x_118 = 0.0f;
          float x_119_phi = 0.0f;
          if ((tint_mod_i32(x_101, 32) == 0)) {
            x_111 = (x_98 + 0.40000000596046447754f);
            x_99_phi = x_111;
          } else {
            x_119_phi = x_98;
            float v_6 = float(x_101);
            float v_7 = round(x_50);
            float v_8 = float(x_101);
            if (((v_6 - (v_7 * floor((v_8 / round(x_50))))) <= 0.00999999977648258209f)) {
              x_118 = (x_98 + 100.0f);
              x_119_phi = x_118;
            }
            x_119 = x_119_phi;
            x_99_phi = x_119;
          }
          x_99 = x_99_phi;
          float v_9 = float(x_101);
          if ((v_9 >= x_92)) {
            x_124_phi = x_99;
            x_125_phi = x_99;
            x_126_phi = true;
            break;
          }
          {
            x_102 = (x_101 + 1);
            x_98_phi = x_99;
            x_101_phi = x_102;
          }
          continue;
        }
      }
      x_124 = x_124_phi;
      x_125 = x_125_phi;
      bool x_126 = x_126_phi;
      x_128_phi = x_124;
      if (x_126) {
        break;
      }
      x_128_phi = x_125;
      break;
    }
  }
  float x_128 = x_128_phi;
  c[1u] = x_128;
  float x_130 = c.x;
  float x_131 = c.y;
  c[2u] = (x_130 + x_131);
  x_135_phi = 0;
  {
    while(true) {
      int x_136 = 0;
      int x_135 = x_135_phi;
      if ((x_135 < 3)) {
      } else {
        break;
      }
      float x_142 = c[x_135];
      if ((x_142 >= 1.0f)) {
        float x_146 = c[x_135];
        float x_147 = c[x_135];
        c[x_135] = (x_146 * x_147);
      }
      {
        x_136 = (x_135 + 1);
        x_135_phi = x_136;
      }
      continue;
    }
  }
  vec3 x_149 = c;
  vec3 x_151 = normalize(abs(x_149));
  x_GLF_color = vec4(x_151[0u], x_151[1u], x_151[2u], 1.0f);
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
