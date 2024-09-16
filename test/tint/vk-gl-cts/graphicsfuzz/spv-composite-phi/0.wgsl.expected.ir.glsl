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
  float x_51 = 0.0f;
  float x_55 = 0.0f;
  float x_56 = 0.0f;
  float x_81 = 0.0f;
  float x_82 = 0.0f;
  float x_118 = 0.0f;
  float x_119 = 0.0f;
  float x_55_phi = 0.0f;
  int x_58_phi = 0;
  float x_81_phi = 0.0f;
  float x_82_phi = 0.0f;
  bool x_83_phi = false;
  float x_85_phi = 0.0f;
  float x_122_phi = 0.0f;
  int x_129_phi = 0;
  c = vec3(7.0f, 8.0f, 9.0f);
  float x_47 = v.tint_symbol_3.resolution.x;
  float x_49 = round((x_47 * 0.125f));
  x_51 = tint_symbol.x;
  switch(0u) {
    default:
    {
      x_55_phi = -0.5f;
      x_58_phi = 1;
      {
        while(true) {
          float x_68 = 0.0f;
          float x_76 = 0.0f;
          int x_59 = 0;
          float x_56_phi = 0.0f;
          x_55 = x_55_phi;
          int x_58 = x_58_phi;
          x_81_phi = 0.0f;
          x_82_phi = x_55;
          x_83_phi = false;
          if ((x_58 < 800)) {
          } else {
            break;
          }
          float x_75 = 0.0f;
          float x_76_phi = 0.0f;
          if ((tint_mod_i32(x_58, 32) == 0)) {
            x_68 = (x_55 + 0.40000000596046447754f);
            x_56_phi = x_68;
          } else {
            x_76_phi = x_55;
            float v_2 = float(x_58);
            float v_3 = round(x_49);
            float v_4 = float(x_58);
            if (((v_2 - (v_3 * floor((v_4 / round(x_49))))) <= 0.00999999977648258209f)) {
              x_75 = (x_55 + 100.0f);
              x_76_phi = x_75;
            }
            x_76 = x_76_phi;
            x_56_phi = x_76;
          }
          x_56 = x_56_phi;
          float v_5 = float(x_58);
          if ((v_5 >= x_51)) {
            x_81_phi = x_56;
            x_82_phi = x_56;
            x_83_phi = true;
            break;
          }
          {
            x_59 = (x_58 + 1);
            x_55_phi = x_56;
            x_58_phi = x_59;
          }
          continue;
        }
      }
      x_81 = x_81_phi;
      x_82 = x_82_phi;
      bool x_83 = x_83_phi;
      x_85_phi = x_81;
      if (x_83) {
        break;
      }
      x_85_phi = x_82;
      break;
    }
  }
  float x_88 = 0.0f;
  float x_92 = 0.0f;
  float x_93 = 0.0f;
  float x_92_phi = 0.0f;
  int x_95_phi = 0;
  float x_118_phi = 0.0f;
  float x_119_phi = 0.0f;
  bool x_120_phi = false;
  float x_85 = x_85_phi;
  c[0u] = x_85;
  x_88 = tint_symbol.y;
  switch(0u) {
    default:
    {
      x_92_phi = -0.5f;
      x_95_phi = 1;
      {
        while(true) {
          float x_105 = 0.0f;
          float x_113 = 0.0f;
          int x_96 = 0;
          float x_93_phi = 0.0f;
          x_92 = x_92_phi;
          int x_95 = x_95_phi;
          x_118_phi = 0.0f;
          x_119_phi = x_92;
          x_120_phi = false;
          if ((x_95 < 800)) {
          } else {
            break;
          }
          float x_112 = 0.0f;
          float x_113_phi = 0.0f;
          if ((tint_mod_i32(x_95, 32) == 0)) {
            x_105 = (x_92 + 0.40000000596046447754f);
            x_93_phi = x_105;
          } else {
            x_113_phi = x_92;
            float v_6 = float(x_95);
            float v_7 = round(x_49);
            float v_8 = float(x_95);
            if (((v_6 - (v_7 * floor((v_8 / round(x_49))))) <= 0.00999999977648258209f)) {
              x_112 = (x_92 + 100.0f);
              x_113_phi = x_112;
            }
            x_113 = x_113_phi;
            x_93_phi = x_113;
          }
          x_93 = x_93_phi;
          float v_9 = float(x_95);
          if ((v_9 >= x_88)) {
            x_118_phi = x_93;
            x_119_phi = x_93;
            x_120_phi = true;
            break;
          }
          {
            x_96 = (x_95 + 1);
            x_92_phi = x_93;
            x_95_phi = x_96;
          }
          continue;
        }
      }
      x_118 = x_118_phi;
      x_119 = x_119_phi;
      bool x_120 = x_120_phi;
      x_122_phi = x_118;
      if (x_120) {
        break;
      }
      x_122_phi = x_119;
      break;
    }
  }
  float x_122 = x_122_phi;
  c[1u] = x_122;
  float x_124 = c.x;
  float x_125 = c.y;
  c[2u] = (x_124 + x_125);
  x_129_phi = 0;
  {
    while(true) {
      int x_130 = 0;
      int x_129 = x_129_phi;
      if ((x_129 < 3)) {
      } else {
        break;
      }
      float x_136 = c[x_129];
      if ((x_136 >= 1.0f)) {
        float x_140 = c[x_129];
        float x_141 = c[x_129];
        c[x_129] = (x_140 * x_141);
      }
      {
        x_130 = (x_129 + 1);
        x_129_phi = x_130;
      }
      continue;
    }
  }
  vec3 x_143 = c;
  vec3 x_145 = normalize(abs(x_143));
  x_GLF_color = vec4(x_145[0u], x_145[1u], x_145[2u], 1.0f);
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
