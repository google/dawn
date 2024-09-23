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
  int x_65 = 0;
  int x_93 = 0;
  int x_102 = 0;
  float x_180 = 0.0f;
  float x_279 = 0.0f;
  float x_280 = 0.0f;
  float x_62 = v.tint_symbol_3.injectionSwitch.x;
  x_65 = tint_f32_to_i32(x_62);
  {
    while(true) {
      int x_66 = 0;
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
      x_66 = (x_65 + 1);
      {
        x_65 = x_66;
        if (!((x_66 < 10))) { break; }
      }
      continue;
    }
  }
  x_93 = 0;
  {
    while(true) {
      int x_94 = 0;
      if ((x_93 < 10)) {
      } else {
        break;
      }
      {
        temp[x_93] = data[x_93];
        x_94 = (x_93 + 1);
        x_93 = x_94;
      }
      continue;
    }
  }
  x_102 = 1;
  {
    while(true) {
      int x_109 = 0;
      int x_103 = 0;
      if ((x_102 <= 9)) {
      } else {
        break;
      }
      x_109 = 0;
      {
        while(true) {
          int x_121 = 0;
          int x_124 = 0;
          int x_126 = 0;
          int x_148 = 0;
          int x_151 = 0;
          int x_161 = 0;
          if ((x_109 < 9)) {
          } else {
            break;
          }
          int x_115 = (x_109 + x_102);
          int x_116 = (x_115 - 1);
          int x_110 = (x_109 + (2 * x_102));
          int x_119 = min((x_110 - 1), 9);
          x_121 = x_109;
          x_124 = x_115;
          x_126 = x_109;
          {
            while(true) {
              int x_141 = 0;
              int x_144 = 0;
              int x_125 = 0;
              int x_127 = 0;
              if (((x_126 <= x_116) & (x_124 <= x_119))) {
              } else {
                break;
              }
              int x_133_save = x_126;
              int x_135_save = x_124;
              int x_122 = (x_121 + 1);
              if ((data[x_126] < data[x_124])) {
                x_141 = (x_126 + 1);
                temp[x_121] = data[x_133_save];
                x_125 = x_124;
                x_127 = x_141;
              } else {
                x_144 = (x_124 + 1);
                temp[x_121] = data[x_135_save];
                x_125 = x_144;
                x_127 = x_126;
              }
              {
                x_121 = x_122;
                x_124 = x_125;
                x_126 = x_127;
              }
              continue;
            }
          }
          x_148 = x_121;
          x_151 = x_126;
          {
            while(true) {
              int x_149 = 0;
              int x_152 = 0;
              if (((x_151 < 10) & (x_151 <= x_116))) {
              } else {
                break;
              }
              {
                x_149 = (x_148 + 1);
                x_152 = (x_151 + 1);
                temp[x_148] = data[x_151];
                x_148 = x_149;
                x_151 = x_152;
              }
              continue;
            }
          }
          x_161 = x_109;
          {
            while(true) {
              int x_162 = 0;
              if ((x_161 <= x_119)) {
              } else {
                break;
              }
              {
                data[x_161] = temp[x_161];
                x_162 = (x_161 + 1);
                x_161 = x_162;
              }
              continue;
            }
          }
          {
            x_109 = x_110;
          }
          continue;
        }
      }
      {
        x_103 = (2 * x_102);
        x_102 = x_103;
      }
      continue;
    }
  }
  int x_171 = 0;
  float x_189 = 0.0f;
  float x_278 = 0.0f;
  x_171 = tint_f32_to_i32(tint_symbol.y);
  if ((x_171 < 30)) {
    x_180 = (0.5f + (float(data[0]) * 0.10000000149011611938f));
    x_280 = x_180;
  } else {
    float x_198 = 0.0f;
    float x_277 = 0.0f;
    if ((x_171 < 60)) {
      x_189 = (0.5f + (float(data[1]) * 0.10000000149011611938f));
      x_279 = x_189;
    } else {
      float x_207 = 0.0f;
      float x_249 = 0.0f;
      if ((x_171 < 90)) {
        x_198 = (0.5f + (float(data[2]) * 0.10000000149011611938f));
        x_278 = x_198;
      } else {
        if ((x_171 < 120)) {
          x_207 = (0.5f + (float(data[3]) * 0.10000000149011611938f));
          x_277 = x_207;
        } else {
          float x_220 = 0.0f;
          float x_248 = 0.0f;
          vec2 x_256 = vec2(0.0f);
          int x_259 = 0;
          if ((x_171 < 150)) {
            continue_execution = false;
          } else {
            float x_229 = 0.0f;
            float x_247 = 0.0f;
            if ((x_171 < 180)) {
              x_220 = (0.5f + (float(data[5]) * 0.10000000149011611938f));
              x_249 = x_220;
            } else {
              float x_238 = 0.0f;
              float x_246 = 0.0f;
              if ((x_171 < 210)) {
                x_229 = (0.5f + (float(data[6]) * 0.10000000149011611938f));
                x_248 = x_229;
              } else {
                if ((x_171 < 240)) {
                  x_238 = (0.5f + (float(data[7]) * 0.10000000149011611938f));
                  x_247 = x_238;
                } else {
                  if ((x_171 < 270)) {
                  } else {
                    continue_execution = false;
                  }
                  x_246 = (0.5f + (float(data[8]) * 0.10000000149011611938f));
                  x_247 = x_246;
                }
                x_248 = x_247;
              }
              x_249 = x_248;
            }
            bool x_252 = (x_62 > v.tint_symbol_3.injectionSwitch.y);
            if (x_252) {
              x_GLF_color = vec4(1.0f);
            }
            x_256 = vec2(1.0f);
            x_259 = 0;
            {
              while(true) {
                vec2 x_272 = vec2(0.0f);
                vec2 x_273 = vec2(0.0f);
                int x_260 = 0;
                if ((x_259 <= 32)) {
                } else {
                  break;
                }
                x_273 = x_256;
                if ((x_256.x < 0.0f)) {
                  if (x_252) {
                    continue_execution = false;
                  }
                  x_272 = x_256;
                  x_272[1u] = (x_256.y + 1.0f);
                  x_273 = x_272;
                }
                vec2 x_257_1 = x_273;
                x_257_1[0u] = (x_273.x + x_273.y);
                vec2 x_257 = x_257_1;
                {
                  x_260 = (x_259 + 1);
                  x_256 = x_257;
                  x_259 = x_260;
                }
                continue;
              }
            }
          }
          x_277 = x_249;
        }
        x_278 = x_277;
      }
      x_279 = x_278;
    }
    x_280 = x_279;
  }
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
ERROR: 0:154: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:154: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
