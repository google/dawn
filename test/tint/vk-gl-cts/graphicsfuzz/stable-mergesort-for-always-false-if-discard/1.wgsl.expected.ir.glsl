SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct buf0 {
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
bool continue_execution = true;
layout(location = 0) out vec4 tint_symbol_1_loc0_Output;
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : ((-2147483647 - 1)))) : (2147483647));
}
void main_1() {
  int temp[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int data[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  float x_180 = 0.0f;
  float x_279 = 0.0f;
  int x_65_phi = 0;
  int x_93_phi = 0;
  int x_102_phi = 0;
  float x_280_phi = 0.0f;
  float x_62 = v.tint_symbol_3.injectionSwitch.x;
  int x_63 = tint_f32_to_i32(x_62);
  x_65_phi = x_63;
  {
    while(true) {
      int x_65 = x_65_phi;
      switch(x_65) {
        case 9:
        {
          data[x_65] = -5;
          break;
        }
        case 8:
        {
          data[x_65] = -4;
          break;
        }
        case 7:
        {
          data[x_65] = -3;
          break;
        }
        case 6:
        {
          data[x_65] = -2;
          break;
        }
        case 5:
        {
          data[x_65] = -1;
          break;
        }
        case 4:
        {
          data[x_65] = 0;
          break;
        }
        case 3:
        {
          data[x_65] = 1;
          break;
        }
        case 2:
        {
          data[x_65] = 2;
          break;
        }
        case 1:
        {
          data[x_65] = 3;
          break;
        }
        case 0:
        {
          data[x_65] = 4;
          break;
        }
        default:
        {
          break;
        }
      }
      int x_66 = (x_65 + 1);
      {
        x_65_phi = x_66;
        if (!((x_66 < 10))) { break; }
      }
      continue;
    }
  }
  x_93_phi = 0;
  {
    while(true) {
      int x_94 = 0;
      int x_93 = x_93_phi;
      if ((x_93 < 10)) {
      } else {
        break;
      }
      {
        int x_99 = data[x_93];
        temp[x_93] = x_99;
        x_94 = (x_93 + 1);
        x_93_phi = x_94;
      }
      continue;
    }
  }
  x_102_phi = 1;
  {
    while(true) {
      int x_103 = 0;
      int x_109_phi = 0;
      int x_102 = x_102_phi;
      if ((x_102 <= 9)) {
      } else {
        break;
      }
      x_109_phi = 0;
      {
        while(true) {
          int x_121 = 0;
          int x_126 = 0;
          int x_121_phi = 0;
          int x_124_phi = 0;
          int x_126_phi = 0;
          int x_148_phi = 0;
          int x_151_phi = 0;
          int x_161_phi = 0;
          int x_109 = x_109_phi;
          if ((x_109 < 9)) {
          } else {
            break;
          }
          int x_115 = (x_109 + x_102);
          int x_116 = (x_115 - 1);
          int x_110 = (x_109 + (2 * x_102));
          int x_119 = min((x_110 - 1), 9);
          x_121_phi = x_109;
          x_124_phi = x_115;
          x_126_phi = x_109;
          {
            while(true) {
              int x_141 = 0;
              int x_144 = 0;
              int x_125_phi = 0;
              int x_127_phi = 0;
              x_121 = x_121_phi;
              int x_124 = x_124_phi;
              x_126 = x_126_phi;
              if (((x_126 <= x_116) & (x_124 <= x_119))) {
              } else {
                break;
              }
              int x_133_save = x_126;
              int x_134 = data[x_133_save];
              int x_135_save = x_124;
              int x_136 = data[x_135_save];
              int x_122 = (x_121 + 1);
              if ((x_134 < x_136)) {
                x_141 = (x_126 + 1);
                int x_142 = data[x_133_save];
                temp[x_121] = x_142;
                x_125_phi = x_124;
                x_127_phi = x_141;
              } else {
                x_144 = (x_124 + 1);
                int x_145 = data[x_135_save];
                temp[x_121] = x_145;
                x_125_phi = x_144;
                x_127_phi = x_126;
              }
              int x_125 = x_125_phi;
              int x_127 = x_127_phi;
              {
                x_121_phi = x_122;
                x_124_phi = x_125;
                x_126_phi = x_127;
              }
              continue;
            }
          }
          x_148_phi = x_121;
          x_151_phi = x_126;
          {
            while(true) {
              int x_149 = 0;
              int x_152 = 0;
              int x_148 = x_148_phi;
              int x_151 = x_151_phi;
              if (((x_151 < 10) & (x_151 <= x_116))) {
              } else {
                break;
              }
              {
                x_149 = (x_148 + 1);
                x_152 = (x_151 + 1);
                int x_158 = data[x_151];
                temp[x_148] = x_158;
                x_148_phi = x_149;
                x_151_phi = x_152;
              }
              continue;
            }
          }
          x_161_phi = x_109;
          {
            while(true) {
              int x_162 = 0;
              int x_161 = x_161_phi;
              if ((x_161 <= x_119)) {
              } else {
                break;
              }
              {
                int x_167 = temp[x_161];
                data[x_161] = x_167;
                x_162 = (x_161 + 1);
                x_161_phi = x_162;
              }
              continue;
            }
          }
          {
            x_109_phi = x_110;
          }
          continue;
        }
      }
      {
        x_103 = (2 * x_102);
        x_102_phi = x_103;
      }
      continue;
    }
  }
  int x_171 = 0;
  float x_189 = 0.0f;
  float x_278 = 0.0f;
  float x_279_phi = 0.0f;
  float x_170 = tint_symbol.y;
  x_171 = tint_f32_to_i32(x_170);
  if ((x_171 < 30)) {
    int x_177 = data[0];
    x_180 = (0.5f + (float(x_177) * 0.10000000149011611938f));
    x_280_phi = x_180;
  } else {
    float x_198 = 0.0f;
    float x_277 = 0.0f;
    float x_278_phi = 0.0f;
    if ((x_171 < 60)) {
      int x_186 = data[1];
      x_189 = (0.5f + (float(x_186) * 0.10000000149011611938f));
      x_279_phi = x_189;
    } else {
      float x_207 = 0.0f;
      float x_249 = 0.0f;
      float x_277_phi = 0.0f;
      if ((x_171 < 90)) {
        int x_195 = data[2];
        x_198 = (0.5f + (float(x_195) * 0.10000000149011611938f));
        x_278_phi = x_198;
      } else {
        if ((x_171 < 120)) {
          int x_204 = data[3];
          x_207 = (0.5f + (float(x_204) * 0.10000000149011611938f));
          x_277_phi = x_207;
        } else {
          float x_220 = 0.0f;
          float x_248 = 0.0f;
          float x_249_phi = 0.0f;
          vec2 x_256_phi = vec2(0.0f);
          int x_259_phi = 0;
          if ((x_171 < 150)) {
            continue_execution = false;
          } else {
            float x_229 = 0.0f;
            float x_247 = 0.0f;
            float x_248_phi = 0.0f;
            if ((x_171 < 180)) {
              int x_217 = data[5];
              x_220 = (0.5f + (float(x_217) * 0.10000000149011611938f));
              x_249_phi = x_220;
            } else {
              float x_238 = 0.0f;
              float x_246 = 0.0f;
              float x_247_phi = 0.0f;
              if ((x_171 < 210)) {
                int x_226 = data[6];
                x_229 = (0.5f + (float(x_226) * 0.10000000149011611938f));
                x_248_phi = x_229;
              } else {
                if ((x_171 < 240)) {
                  int x_235 = data[7];
                  x_238 = (0.5f + (float(x_235) * 0.10000000149011611938f));
                  x_247_phi = x_238;
                } else {
                  if ((x_171 < 270)) {
                  } else {
                    continue_execution = false;
                  }
                  int x_243 = data[8];
                  x_246 = (0.5f + (float(x_243) * 0.10000000149011611938f));
                  x_247_phi = x_246;
                }
                x_247 = x_247_phi;
                x_248_phi = x_247;
              }
              x_248 = x_248_phi;
              x_249_phi = x_248;
            }
            x_249 = x_249_phi;
            float x_251 = v.tint_symbol_3.injectionSwitch.y;
            bool x_252 = (x_62 > x_251);
            if (x_252) {
              x_GLF_color = vec4(1.0f);
            }
            x_256_phi = vec2(1.0f);
            x_259_phi = 0;
            {
              while(true) {
                vec2 x_272 = vec2(0.0f);
                int x_260 = 0;
                vec2 x_273_phi = vec2(0.0f);
                vec2 x_256 = x_256_phi;
                int x_259 = x_259_phi;
                if ((x_259 <= 32)) {
                } else {
                  break;
                }
                x_273_phi = x_256;
                if ((x_256[0u] < 0.0f)) {
                  if (x_252) {
                    continue_execution = false;
                  }
                  x_272 = x_256;
                  x_272[1u] = (x_256[1u] + 1.0f);
                  x_273_phi = x_272;
                }
                vec2 x_273 = x_273_phi;
                vec2 x_257_1 = x_273;
                x_257_1[0u] = (x_273[0u] + x_273[1u]);
                vec2 x_257 = x_257_1;
                {
                  x_260 = (x_259 + 1);
                  x_256_phi = x_257;
                  x_259_phi = x_260;
                }
                continue;
              }
            }
          }
          x_277_phi = x_249;
        }
        x_277 = x_277_phi;
        x_278_phi = x_277;
      }
      x_278 = x_278_phi;
      x_279_phi = x_278;
    }
    x_279 = x_279_phi;
    x_280_phi = x_279;
  }
  float x_280 = x_280_phi;
  x_GLF_color = vec4(x_280, x_280, x_280, 1.0f);
}
main_out tint_symbol_1_inner(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  main_out v_1 = main_out(x_GLF_color);
  if (!(continue_execution)) {
    discard;
  }
  return v_1;
}
void main() {
  tint_symbol_1_loc0_Output = tint_symbol_1_inner(gl_FragCoord).x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:164: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:164: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
