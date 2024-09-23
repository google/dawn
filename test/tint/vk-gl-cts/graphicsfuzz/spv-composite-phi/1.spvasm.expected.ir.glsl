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
  int x_60 = 0;
  float x_58 = 0.0f;
  float x_83 = 0.0f;
  float x_84 = 0.0f;
  bool x_85 = false;
  float x_87 = 0.0f;
  float x_124 = 0.0f;
  float x_125 = 0.0f;
  float x_128 = 0.0f;
  int x_135 = 0;
  c = vec3(7.0f, 8.0f, 9.0f);
  float x_47 = v.tint_symbol_3.resolution.x;
  vec2 x_48 = vec2(1.0f, x_47);
  float x_50 = round((x_47 * 0.125f));
  vec2 x_51 = vec2(0.0f, -0.5f);
  x_53 = tint_symbol.x;
  switch(0u) {
    default:
    {
      x_57 = -0.5f;
      x_60 = 1;
      {
        while(true) {
          float x_70 = 0.0f;
          float x_78 = 0.0f;
          int x_61 = 0;
          x_83 = 0.0f;
          x_84 = x_57;
          x_85 = false;
          if ((x_60 < 800)) {
          } else {
            break;
          }
          float x_77 = 0.0f;
          if ((tint_mod_i32(x_60, 32) == 0)) {
            x_70 = (x_57 + 0.40000000596046447754f);
            x_58 = x_70;
          } else {
            x_78 = x_57;
            float v_2 = float(x_60);
            float v_3 = round(x_50);
            float v_4 = float(x_60);
            if (((v_2 - (v_3 * floor((v_4 / round(x_50))))) <= 0.00999999977648258209f)) {
              x_77 = (x_57 + 100.0f);
              x_78 = x_77;
            }
            x_58 = x_78;
          }
          float v_5 = float(x_60);
          if ((v_5 >= x_53)) {
            x_83 = x_58;
            x_84 = x_58;
            x_85 = true;
            break;
          }
          {
            x_61 = (x_60 + 1);
            x_57 = x_58;
            x_60 = x_61;
          }
          continue;
        }
      }
      x_87 = x_83;
      if (x_85) {
        break;
      }
      x_87 = x_84;
      break;
    }
  }
  float x_92 = 0.0f;
  float x_98 = 0.0f;
  int x_101 = 0;
  float x_99 = 0.0f;
  bool x_126 = false;
  vec4 x_89 = vec4(x_84, 0.40000000596046447754f, x_83, 0.40000000596046447754f);
  c[0u] = x_87;
  x_92 = tint_symbol.y;
  switch(0u) {
    default:
    {
      vec4 x_95 = vec4(x_51, 0.0f, x_57);
      x_98 = vec3(x_48, -0.5f)[2u];
      x_101 = 1;
      {
        while(true) {
          float x_111 = 0.0f;
          float x_119 = 0.0f;
          int x_102 = 0;
          x_124 = 0.0f;
          x_125 = x_98;
          x_126 = false;
          if ((x_101 < 800)) {
          } else {
            break;
          }
          float x_118 = 0.0f;
          if ((tint_mod_i32(x_101, 32) == 0)) {
            x_111 = (x_98 + 0.40000000596046447754f);
            x_99 = x_111;
          } else {
            x_119 = x_98;
            float v_6 = float(x_101);
            float v_7 = round(x_50);
            float v_8 = float(x_101);
            if (((v_6 - (v_7 * floor((v_8 / round(x_50))))) <= 0.00999999977648258209f)) {
              x_118 = (x_98 + 100.0f);
              x_119 = x_118;
            }
            x_99 = x_119;
          }
          float v_9 = float(x_101);
          if ((v_9 >= x_92)) {
            x_124 = x_99;
            x_125 = x_99;
            x_126 = true;
            break;
          }
          {
            x_102 = (x_101 + 1);
            x_98 = x_99;
            x_101 = x_102;
          }
          continue;
        }
      }
      x_128 = x_124;
      if (x_126) {
        break;
      }
      x_128 = x_125;
      break;
    }
  }
  c[1u] = x_128;
  c[2u] = (c.x + c.y);
  x_135 = 0;
  {
    while(true) {
      int x_136 = 0;
      if ((x_135 < 3)) {
      } else {
        break;
      }
      if ((c[x_135] >= 1.0f)) {
        c[x_135] = (c[x_135] * c[x_135]);
      }
      {
        x_136 = (x_135 + 1);
        x_135 = x_136;
      }
      continue;
    }
  }
  vec3 x_151 = normalize(abs(c));
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
