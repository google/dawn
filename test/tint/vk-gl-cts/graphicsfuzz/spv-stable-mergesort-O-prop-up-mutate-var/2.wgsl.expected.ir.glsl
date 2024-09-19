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
  float x_189 = 0.0f;
  float x_261 = 0.0f;
  int x_63_phi = 0;
  int x_102_phi = 0;
  int x_111_phi = 0;
  float x_262_phi = 0.0f;
  float x_60 = v.tint_symbol_3.injectionSwitch.x;
  int x_61 = tint_f32_to_i32(x_60);
  x_63_phi = x_61;
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
      int x_64_phi = 0;
      int x_63 = x_63_phi;
      int x_68 = (x_63 + 1);
      x_64_phi = x_68;
      switch(x_63) {
        case 9:
        {
          data[x_63] = -5;
          x_99 = (x_63 + 1);
          x_64_phi = x_99;
          break;
        }
        case 8:
        {
          data[x_63] = -4;
          x_97 = (x_63 + 1);
          x_64_phi = x_97;
          break;
        }
        case 7:
        {
          data[x_63] = -3;
          x_95 = (x_63 + 1);
          x_64_phi = x_95;
          break;
        }
        case 6:
        {
          data[x_63] = -2;
          x_93 = (x_63 + 1);
          x_64_phi = x_93;
          break;
        }
        case 5:
        {
          data[x_63] = -1;
          x_91 = (x_63 + 1);
          x_64_phi = x_91;
          break;
        }
        case 4:
        {
          data[x_63] = 0;
          x_89 = (x_63 + 1);
          x_64_phi = x_89;
          break;
        }
        case 3:
        {
          data[x_63] = 1;
          x_87 = (x_63 + 1);
          x_64_phi = x_87;
          break;
        }
        case 2:
        {
          data[x_63] = 2;
          x_85 = (x_63 + 1);
          x_64_phi = x_85;
          break;
        }
        case 1:
        {
          data[x_63] = 3;
          x_83 = (x_63 + 1);
          x_64_phi = x_83;
          break;
        }
        case 0:
        {
          data[x_63] = 4;
          x_81 = (x_63 + 1);
          x_64_phi = x_81;
          break;
        }
        default:
        {
          break;
        }
      }
      int x_64 = x_64_phi;
      {
        x_63_phi = x_64;
        if (!((x_64 < 10))) { break; }
      }
      continue;
    }
  }
  x_102_phi = 0;
  {
    while(true) {
      int x_103 = 0;
      int x_102 = x_102_phi;
      if ((x_102 < 10)) {
      } else {
        break;
      }
      {
        int x_108 = data[x_102];
        temp[x_102] = x_108;
        x_103 = (x_102 + 1);
        x_102_phi = x_103;
      }
      continue;
    }
  }
  x_111_phi = 1;
  {
    while(true) {
      int x_112 = 0;
      int x_118_phi = 0;
      int x_111 = x_111_phi;
      if ((x_111 <= 9)) {
      } else {
        break;
      }
      x_118_phi = 0;
      {
        while(true) {
          int x_130 = 0;
          int x_135 = 0;
          int x_130_phi = 0;
          int x_133_phi = 0;
          int x_135_phi = 0;
          int x_157_phi = 0;
          int x_160_phi = 0;
          int x_170_phi = 0;
          int x_118 = x_118_phi;
          if ((x_118 < 9)) {
          } else {
            break;
          }
          int x_124 = (x_118 + x_111);
          int x_125 = (x_124 - 1);
          int x_119 = (x_118 + (2 * x_111));
          int x_128 = min((x_119 - 1), 9);
          x_130_phi = x_118;
          x_133_phi = x_124;
          x_135_phi = x_118;
          {
            while(true) {
              int x_150 = 0;
              int x_153 = 0;
              int x_134_phi = 0;
              int x_136_phi = 0;
              x_130 = x_130_phi;
              int x_133 = x_133_phi;
              x_135 = x_135_phi;
              if (((x_135 <= x_125) & (x_133 <= x_128))) {
              } else {
                break;
              }
              int x_142_save = x_135;
              int x_143 = data[x_142_save];
              int x_144_save = x_133;
              int x_145 = data[x_144_save];
              int x_131 = (x_130 + 1);
              if ((x_143 < x_145)) {
                x_150 = (x_135 + 1);
                int x_151 = data[x_142_save];
                temp[x_130] = x_151;
                x_134_phi = x_133;
                x_136_phi = x_150;
              } else {
                x_153 = (x_133 + 1);
                int x_154 = data[x_144_save];
                temp[x_130] = x_154;
                x_134_phi = x_153;
                x_136_phi = x_135;
              }
              int x_134 = x_134_phi;
              int x_136 = x_136_phi;
              {
                x_130_phi = x_131;
                x_133_phi = x_134;
                x_135_phi = x_136;
              }
              continue;
            }
          }
          x_157_phi = x_130;
          x_160_phi = x_135;
          {
            while(true) {
              int x_158 = 0;
              int x_161 = 0;
              int x_157 = x_157_phi;
              int x_160 = x_160_phi;
              if (((x_160 < 10) & (x_160 <= x_125))) {
              } else {
                break;
              }
              {
                x_158 = (x_157 + 1);
                x_161 = (x_160 + 1);
                int x_167 = data[x_160];
                temp[x_157] = x_167;
                x_157_phi = x_158;
                x_160_phi = x_161;
              }
              continue;
            }
          }
          x_170_phi = x_118;
          {
            while(true) {
              int x_171 = 0;
              int x_170 = x_170_phi;
              if ((x_170 <= x_128)) {
              } else {
                break;
              }
              {
                int x_176 = temp[x_170];
                data[x_170] = x_176;
                x_171 = (x_170 + 1);
                x_170_phi = x_171;
              }
              continue;
            }
          }
          {
            x_118_phi = x_119;
          }
          continue;
        }
      }
      {
        x_112 = (2 * x_111);
        x_111_phi = x_112;
      }
      continue;
    }
  }
  int x_180 = 0;
  float x_198 = 0.0f;
  float x_260 = 0.0f;
  float x_261_phi = 0.0f;
  float x_179 = tint_symbol.y;
  x_180 = tint_f32_to_i32(x_179);
  if ((x_180 < 30)) {
    int x_186 = data[0];
    x_189 = (0.5f + (float(x_186) * 0.10000000149011611938f));
    x_262_phi = x_189;
  } else {
    float x_207 = 0.0f;
    float x_259 = 0.0f;
    float x_260_phi = 0.0f;
    if ((x_180 < 60)) {
      int x_195 = data[1];
      x_198 = (0.5f + (float(x_195) * 0.10000000149011611938f));
      x_261_phi = x_198;
    } else {
      float x_216 = 0.0f;
      float x_258 = 0.0f;
      float x_259_phi = 0.0f;
      if ((x_180 < 90)) {
        int x_204 = data[2];
        x_207 = (0.5f + (float(x_204) * 0.10000000149011611938f));
        x_260_phi = x_207;
      } else {
        if ((x_180 < 120)) {
          int x_213 = data[3];
          x_216 = (0.5f + (float(x_213) * 0.10000000149011611938f));
          x_259_phi = x_216;
        } else {
          float x_229 = 0.0f;
          float x_257 = 0.0f;
          float x_258_phi = 0.0f;
          if ((x_180 < 150)) {
            continue_execution = false;
          } else {
            float x_238 = 0.0f;
            float x_256 = 0.0f;
            float x_257_phi = 0.0f;
            if ((x_180 < 180)) {
              int x_226 = data[5];
              x_229 = (0.5f + (float(x_226) * 0.10000000149011611938f));
              x_258_phi = x_229;
            } else {
              float x_247 = 0.0f;
              float x_255 = 0.0f;
              float x_256_phi = 0.0f;
              if ((x_180 < 210)) {
                int x_235 = data[6];
                x_238 = (0.5f + (float(x_235) * 0.10000000149011611938f));
                x_257_phi = x_238;
              } else {
                if ((x_180 < 240)) {
                  int x_244 = data[7];
                  x_247 = (0.5f + (float(x_244) * 0.10000000149011611938f));
                  x_256_phi = x_247;
                } else {
                  if ((x_180 < 270)) {
                  } else {
                    continue_execution = false;
                  }
                  int x_252 = data[8];
                  x_255 = (0.5f + (float(x_252) * 0.10000000149011611938f));
                  x_256_phi = x_255;
                }
                x_256 = x_256_phi;
                x_257_phi = x_256;
              }
              x_257 = x_257_phi;
              x_258_phi = x_257;
            }
            x_258 = x_258_phi;
          }
          x_259_phi = x_258;
        }
        x_259 = x_259_phi;
        x_260_phi = x_259;
      }
      x_260 = x_260_phi;
      x_261_phi = x_260;
    }
    x_261 = x_261_phi;
    x_262_phi = x_261;
  }
  float x_262 = x_262_phi;
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
ERROR: 0:197: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:197: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
