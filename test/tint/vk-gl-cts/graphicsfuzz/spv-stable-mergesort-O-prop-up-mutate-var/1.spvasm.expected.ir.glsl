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
  int x_63 = 0;
  int x_103 = 0;
  int x_112 = 0;
  float x_190 = 0.0f;
  float x_262 = 0.0f;
  float x_263 = 0.0f;
  x_63 = tint_f32_to_i32(v.tint_symbol_3.injectionSwitch.x);
  {
    while(true) {
      int x_100 = 0;
      int x_98 = 0;
      int x_96 = 0;
      int x_94 = 0;
      int x_92 = 0;
      int x_90 = 0;
      int x_88 = 0;
      int x_86 = 0;
      int x_84 = 0;
      int x_82 = 0;
      int x_64 = 0;
      int x_68[10] = data;
      data = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
      data = x_68;
      x_64 = (x_63 + 1);
      switch(x_63) {
        case 9:
        {
          data[x_63] = -5;
          x_100 = (x_63 + 1);
          x_64 = x_100;
          break;
        }
        case 8:
        {
          data[x_63] = -4;
          x_98 = (x_63 + 1);
          x_64 = x_98;
          break;
        }
        case 7:
        {
          data[x_63] = -3;
          x_96 = (x_63 + 1);
          x_64 = x_96;
          break;
        }
        case 6:
        {
          data[x_63] = -2;
          x_94 = (x_63 + 1);
          x_64 = x_94;
          break;
        }
        case 5:
        {
          data[x_63] = -1;
          x_92 = (x_63 + 1);
          x_64 = x_92;
          break;
        }
        case 4:
        {
          data[x_63] = 0;
          x_90 = (x_63 + 1);
          x_64 = x_90;
          break;
        }
        case 3:
        {
          data[x_63] = 1;
          x_88 = (x_63 + 1);
          x_64 = x_88;
          break;
        }
        case 2:
        {
          data[x_63] = 2;
          x_86 = (x_63 + 1);
          x_64 = x_86;
          break;
        }
        case 1:
        {
          data[x_63] = 3;
          x_84 = (x_63 + 1);
          x_64 = x_84;
          break;
        }
        case 0:
        {
          data[x_63] = 4;
          x_82 = (x_63 + 1);
          x_64 = x_82;
          break;
        }
        default:
        {
          break;
        }
      }
      {
        x_63 = x_64;
        if (!((x_64 < 10))) { break; }
      }
      continue;
    }
  }
  x_103 = 0;
  {
    while(true) {
      int x_104 = 0;
      if ((x_103 < 10)) {
      } else {
        break;
      }
      {
        temp[x_103] = data[x_103];
        x_104 = (x_103 + 1);
        x_103 = x_104;
      }
      continue;
    }
  }
  x_112 = 1;
  {
    while(true) {
      int x_119 = 0;
      int x_113 = 0;
      if ((x_112 <= 9)) {
      } else {
        break;
      }
      x_119 = 0;
      {
        while(true) {
          int x_131 = 0;
          int x_134 = 0;
          int x_136 = 0;
          int x_158 = 0;
          int x_161 = 0;
          int x_171 = 0;
          if ((x_119 < 9)) {
          } else {
            break;
          }
          int x_125 = (x_119 + x_112);
          int x_126 = (x_125 - 1);
          int x_120 = (x_119 + (2 * x_112));
          int x_129 = min((x_120 - 1), 9);
          x_131 = x_119;
          x_134 = x_125;
          x_136 = x_119;
          {
            while(true) {
              int x_151 = 0;
              int x_154 = 0;
              int x_135 = 0;
              int x_137 = 0;
              if (((x_136 <= x_126) & (x_134 <= x_129))) {
              } else {
                break;
              }
              int x_143_save = x_136;
              int x_145_save = x_134;
              int x_132 = (x_131 + 1);
              if ((data[x_136] < data[x_134])) {
                x_151 = (x_136 + 1);
                temp[x_131] = data[x_143_save];
                x_135 = x_134;
                x_137 = x_151;
              } else {
                x_154 = (x_134 + 1);
                temp[x_131] = data[x_145_save];
                x_135 = x_154;
                x_137 = x_136;
              }
              {
                x_131 = x_132;
                x_134 = x_135;
                x_136 = x_137;
              }
              continue;
            }
          }
          x_158 = x_131;
          x_161 = x_136;
          {
            while(true) {
              int x_159 = 0;
              int x_162 = 0;
              if (((x_161 < 10) & (x_161 <= x_126))) {
              } else {
                break;
              }
              {
                x_159 = (x_158 + 1);
                x_162 = (x_161 + 1);
                temp[x_158] = data[x_161];
                x_158 = x_159;
                x_161 = x_162;
              }
              continue;
            }
          }
          x_171 = x_119;
          {
            while(true) {
              int x_172 = 0;
              if ((x_171 <= x_129)) {
              } else {
                break;
              }
              {
                data[x_171] = temp[x_171];
                x_172 = (x_171 + 1);
                x_171 = x_172;
              }
              continue;
            }
          }
          {
            x_119 = x_120;
          }
          continue;
        }
      }
      {
        x_113 = (2 * x_112);
        x_112 = x_113;
      }
      continue;
    }
  }
  int x_181 = 0;
  float x_199 = 0.0f;
  float x_261 = 0.0f;
  x_181 = tint_f32_to_i32(tint_symbol.y);
  if ((x_181 < 30)) {
    x_190 = (0.5f + (float(data[0]) * 0.10000000149011611938f));
    x_263 = x_190;
  } else {
    float x_208 = 0.0f;
    float x_260 = 0.0f;
    if ((x_181 < 60)) {
      x_199 = (0.5f + (float(data[1]) * 0.10000000149011611938f));
      x_262 = x_199;
    } else {
      float x_217 = 0.0f;
      float x_259 = 0.0f;
      if ((x_181 < 90)) {
        x_208 = (0.5f + (float(data[2]) * 0.10000000149011611938f));
        x_261 = x_208;
      } else {
        if ((x_181 < 120)) {
          x_217 = (0.5f + (float(data[3]) * 0.10000000149011611938f));
          x_260 = x_217;
        } else {
          float x_230 = 0.0f;
          float x_258 = 0.0f;
          if ((x_181 < 150)) {
            continue_execution = false;
          } else {
            float x_239 = 0.0f;
            float x_257 = 0.0f;
            if ((x_181 < 180)) {
              x_230 = (0.5f + (float(data[5]) * 0.10000000149011611938f));
              x_259 = x_230;
            } else {
              float x_248 = 0.0f;
              float x_256 = 0.0f;
              if ((x_181 < 210)) {
                x_239 = (0.5f + (float(data[6]) * 0.10000000149011611938f));
                x_258 = x_239;
              } else {
                if ((x_181 < 240)) {
                  x_248 = (0.5f + (float(data[7]) * 0.10000000149011611938f));
                  x_257 = x_248;
                } else {
                  if ((x_181 < 270)) {
                  } else {
                    continue_execution = false;
                  }
                  x_256 = (0.5f + (float(data[8]) * 0.10000000149011611938f));
                  x_257 = x_256;
                }
                x_258 = x_257;
              }
              x_259 = x_258;
            }
          }
          x_260 = x_259;
        }
        x_261 = x_260;
      }
      x_262 = x_261;
    }
    x_263 = x_262;
  }
  x_GLF_color = vec4(x_263, x_263, x_263, 1.0f);
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
ERROR: 0:186: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:186: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
