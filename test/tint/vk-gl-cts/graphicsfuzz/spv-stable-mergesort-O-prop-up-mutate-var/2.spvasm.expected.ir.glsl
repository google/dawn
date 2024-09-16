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
  int x_102 = 0;
  int x_111 = 0;
  float x_189 = 0.0f;
  float x_261 = 0.0f;
  float x_262 = 0.0f;
  x_63 = tint_f32_to_i32(v.tint_symbol_3.injectionSwitch.x);
  {
    while(true) {
      int x_99 = 0;
      int x_97 = 0;
      int x_95 = 0;
      int x_93 = 0;
      int x_91 = 0;
      int x_89 = 0;
      int x_87 = 0;
      int x_85 = 0;
      int x_83 = 0;
      int x_81 = 0;
      int x_64 = 0;
      x_64 = (x_63 + 1);
      switch(x_63) {
        case 9:
        {
          data[x_63] = -5;
          x_99 = (x_63 + 1);
          x_64 = x_99;
          break;
        }
        case 8:
        {
          data[x_63] = -4;
          x_97 = (x_63 + 1);
          x_64 = x_97;
          break;
        }
        case 7:
        {
          data[x_63] = -3;
          x_95 = (x_63 + 1);
          x_64 = x_95;
          break;
        }
        case 6:
        {
          data[x_63] = -2;
          x_93 = (x_63 + 1);
          x_64 = x_93;
          break;
        }
        case 5:
        {
          data[x_63] = -1;
          x_91 = (x_63 + 1);
          x_64 = x_91;
          break;
        }
        case 4:
        {
          data[x_63] = 0;
          x_89 = (x_63 + 1);
          x_64 = x_89;
          break;
        }
        case 3:
        {
          data[x_63] = 1;
          x_87 = (x_63 + 1);
          x_64 = x_87;
          break;
        }
        case 2:
        {
          data[x_63] = 2;
          x_85 = (x_63 + 1);
          x_64 = x_85;
          break;
        }
        case 1:
        {
          data[x_63] = 3;
          x_83 = (x_63 + 1);
          x_64 = x_83;
          break;
        }
        case 0:
        {
          data[x_63] = 4;
          x_81 = (x_63 + 1);
          x_64 = x_81;
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
  x_102 = 0;
  {
    while(true) {
      int x_103 = 0;
      if ((x_102 < 10)) {
      } else {
        break;
      }
      {
        temp[x_102] = data[x_102];
        x_103 = (x_102 + 1);
        x_102 = x_103;
      }
      continue;
    }
  }
  x_111 = 1;
  {
    while(true) {
      int x_118 = 0;
      int x_112 = 0;
      if ((x_111 <= 9)) {
      } else {
        break;
      }
      x_118 = 0;
      {
        while(true) {
          int x_130 = 0;
          int x_133 = 0;
          int x_135 = 0;
          int x_157 = 0;
          int x_160 = 0;
          int x_170 = 0;
          if ((x_118 < 9)) {
          } else {
            break;
          }
          int x_124 = (x_118 + x_111);
          int x_125 = (x_124 - 1);
          int x_119 = (x_118 + (2 * x_111));
          int x_128 = min((x_119 - 1), 9);
          x_130 = x_118;
          x_133 = x_124;
          x_135 = x_118;
          {
            while(true) {
              int x_150 = 0;
              int x_153 = 0;
              int x_134 = 0;
              int x_136 = 0;
              if (((x_135 <= x_125) & (x_133 <= x_128))) {
              } else {
                break;
              }
              int x_142_save = x_135;
              int x_144_save = x_133;
              int x_131 = (x_130 + 1);
              if ((data[x_135] < data[x_133])) {
                x_150 = (x_135 + 1);
                temp[x_130] = data[x_142_save];
                x_134 = x_133;
                x_136 = x_150;
              } else {
                x_153 = (x_133 + 1);
                temp[x_130] = data[x_144_save];
                x_134 = x_153;
                x_136 = x_135;
              }
              {
                x_130 = x_131;
                x_133 = x_134;
                x_135 = x_136;
              }
              continue;
            }
          }
          x_157 = x_130;
          x_160 = x_135;
          {
            while(true) {
              int x_158 = 0;
              int x_161 = 0;
              if (((x_160 < 10) & (x_160 <= x_125))) {
              } else {
                break;
              }
              {
                x_158 = (x_157 + 1);
                x_161 = (x_160 + 1);
                temp[x_157] = data[x_160];
                x_157 = x_158;
                x_160 = x_161;
              }
              continue;
            }
          }
          x_170 = x_118;
          {
            while(true) {
              int x_171 = 0;
              if ((x_170 <= x_128)) {
              } else {
                break;
              }
              {
                data[x_170] = temp[x_170];
                x_171 = (x_170 + 1);
                x_170 = x_171;
              }
              continue;
            }
          }
          {
            x_118 = x_119;
          }
          continue;
        }
      }
      {
        x_112 = (2 * x_111);
        x_111 = x_112;
      }
      continue;
    }
  }
  int x_180 = 0;
  float x_198 = 0.0f;
  float x_260 = 0.0f;
  x_180 = tint_f32_to_i32(tint_symbol.y);
  if ((x_180 < 30)) {
    x_189 = (0.5f + (float(data[0]) * 0.10000000149011611938f));
    x_262 = x_189;
  } else {
    float x_207 = 0.0f;
    float x_259 = 0.0f;
    if ((x_180 < 60)) {
      x_198 = (0.5f + (float(data[1]) * 0.10000000149011611938f));
      x_261 = x_198;
    } else {
      float x_216 = 0.0f;
      float x_258 = 0.0f;
      if ((x_180 < 90)) {
        x_207 = (0.5f + (float(data[2]) * 0.10000000149011611938f));
        x_260 = x_207;
      } else {
        if ((x_180 < 120)) {
          x_216 = (0.5f + (float(data[3]) * 0.10000000149011611938f));
          x_259 = x_216;
        } else {
          float x_229 = 0.0f;
          float x_257 = 0.0f;
          if ((x_180 < 150)) {
            continue_execution = false;
          } else {
            float x_238 = 0.0f;
            float x_256 = 0.0f;
            if ((x_180 < 180)) {
              x_229 = (0.5f + (float(data[5]) * 0.10000000149011611938f));
              x_258 = x_229;
            } else {
              float x_247 = 0.0f;
              float x_255 = 0.0f;
              if ((x_180 < 210)) {
                x_238 = (0.5f + (float(data[6]) * 0.10000000149011611938f));
                x_257 = x_238;
              } else {
                if ((x_180 < 240)) {
                  x_247 = (0.5f + (float(data[7]) * 0.10000000149011611938f));
                  x_256 = x_247;
                } else {
                  if ((x_180 < 270)) {
                  } else {
                    continue_execution = false;
                  }
                  x_255 = (0.5f + (float(data[8]) * 0.10000000149011611938f));
                  x_256 = x_255;
                }
                x_257 = x_256;
              }
              x_258 = x_257;
            }
          }
          x_259 = x_258;
        }
        x_260 = x_259;
      }
      x_261 = x_260;
    }
    x_262 = x_261;
  }
  x_GLF_color = vec4(x_262, x_262, x_262, 1.0f);
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
ERROR: 0:183: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:183: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
