cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int temp[10] = (int[10])0;
  int data[10] = (int[10])0;
  float x_189 = 0.0f;
  float x_261 = 0.0f;
  int x_63_phi = 0;
  int x_102_phi = 0;
  int x_111_phi = 0;
  float x_262_phi = 0.0f;
  const float x_60 = asfloat(x_8[0].x);
  const int x_61 = int(x_60);
  x_63_phi = x_61;
  while (true) {
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
    const int x_63 = x_63_phi;
    const int x_68 = (x_63 + 1);
    x_64_phi = x_68;
    switch(x_63) {
      case 9: {
        data[x_63] = -5;
        x_99 = (x_63 + 1);
        x_64_phi = x_99;
        break;
      }
      case 8: {
        data[x_63] = -4;
        x_97 = (x_63 + 1);
        x_64_phi = x_97;
        break;
      }
      case 7: {
        data[x_63] = -3;
        x_95 = (x_63 + 1);
        x_64_phi = x_95;
        break;
      }
      case 6: {
        data[x_63] = -2;
        x_93 = (x_63 + 1);
        x_64_phi = x_93;
        break;
      }
      case 5: {
        data[x_63] = -1;
        x_91 = (x_63 + 1);
        x_64_phi = x_91;
        break;
      }
      case 4: {
        data[x_63] = 0;
        x_89 = (x_63 + 1);
        x_64_phi = x_89;
        break;
      }
      case 3: {
        data[x_63] = 1;
        x_87 = (x_63 + 1);
        x_64_phi = x_87;
        break;
      }
      case 2: {
        data[x_63] = 2;
        x_85 = (x_63 + 1);
        x_64_phi = x_85;
        break;
      }
      case 1: {
        data[x_63] = 3;
        x_83 = (x_63 + 1);
        x_64_phi = x_83;
        break;
      }
      case 0: {
        data[x_63] = 4;
        x_81 = (x_63 + 1);
        x_64_phi = x_81;
        break;
      }
      default: {
        break;
      }
    }
    const int x_64 = x_64_phi;
    {
      x_63_phi = x_64;
      if ((x_64 < 10)) {
      } else {
        break;
      }
    }
  }
  x_102_phi = 0;
  while (true) {
    int x_103 = 0;
    const int x_102 = x_102_phi;
    if ((x_102 < 10)) {
    } else {
      break;
    }
    {
      const int x_108 = data[x_102];
      temp[x_102] = x_108;
      x_103 = (x_102 + 1);
      x_102_phi = x_103;
    }
  }
  x_111_phi = 1;
  while (true) {
    int x_112 = 0;
    int x_118_phi = 0;
    const int x_111 = x_111_phi;
    if ((x_111 <= 9)) {
    } else {
      break;
    }
    x_118_phi = 0;
    while (true) {
      int x_130 = 0;
      int x_135 = 0;
      int x_130_phi = 0;
      int x_133_phi = 0;
      int x_135_phi = 0;
      int x_157_phi = 0;
      int x_160_phi = 0;
      int x_170_phi = 0;
      const int x_118 = x_118_phi;
      if ((x_118 < 9)) {
      } else {
        break;
      }
      const int x_124 = (x_118 + x_111);
      const int x_125 = (x_124 - 1);
      const int x_119 = (x_118 + (2 * x_111));
      const int x_128 = min((x_119 - 1), 9);
      x_130_phi = x_118;
      x_133_phi = x_124;
      x_135_phi = x_118;
      while (true) {
        int x_150 = 0;
        int x_153 = 0;
        int x_134_phi = 0;
        int x_136_phi = 0;
        x_130 = x_130_phi;
        const int x_133 = x_133_phi;
        x_135 = x_135_phi;
        if (((x_135 <= x_125) & (x_133 <= x_128))) {
        } else {
          break;
        }
        const int x_142_save = x_135;
        const int x_143 = data[x_142_save];
        const int x_144_save = x_133;
        const int x_145 = data[x_144_save];
        const int x_131 = asint((x_130 + asint(1)));
        if ((x_143 < x_145)) {
          x_150 = asint((x_135 + asint(1)));
          const int x_151 = data[x_142_save];
          temp[x_130] = x_151;
          x_134_phi = x_133;
          x_136_phi = x_150;
        } else {
          x_153 = (x_133 + 1);
          const int x_154 = data[x_144_save];
          temp[x_130] = x_154;
          x_134_phi = x_153;
          x_136_phi = x_135;
        }
        const int x_134 = x_134_phi;
        const int x_136 = x_136_phi;
        {
          x_130_phi = x_131;
          x_133_phi = x_134;
          x_135_phi = x_136;
        }
      }
      x_157_phi = x_130;
      x_160_phi = x_135;
      while (true) {
        int x_158 = 0;
        int x_161 = 0;
        const int x_157 = x_157_phi;
        const int x_160 = x_160_phi;
        if (((x_160 < 10) & (x_160 <= x_125))) {
        } else {
          break;
        }
        {
          x_158 = (x_157 + 1);
          x_161 = (x_160 + 1);
          const int x_167 = data[x_160];
          temp[x_157] = x_167;
          x_157_phi = x_158;
          x_160_phi = x_161;
        }
      }
      x_170_phi = x_118;
      while (true) {
        int x_171 = 0;
        const int x_170 = x_170_phi;
        if ((x_170 <= x_128)) {
        } else {
          break;
        }
        {
          const int x_176 = temp[x_170];
          data[x_170] = x_176;
          x_171 = (x_170 + 1);
          x_170_phi = x_171;
        }
      }
      {
        x_118_phi = x_119;
      }
    }
    {
      x_112 = (2 * x_111);
      x_111_phi = x_112;
    }
  }
  int x_180 = 0;
  float x_198 = 0.0f;
  float x_260 = 0.0f;
  float x_261_phi = 0.0f;
  const float x_179 = gl_FragCoord.y;
  x_180 = int(x_179);
  if ((x_180 < 30)) {
    const int x_186 = data[0];
    x_189 = (0.5f + (float(x_186) * 0.100000001f));
    x_262_phi = x_189;
  } else {
    float x_207 = 0.0f;
    float x_259 = 0.0f;
    float x_260_phi = 0.0f;
    if ((x_180 < 60)) {
      const int x_195 = data[1];
      x_198 = (0.5f + (float(x_195) * 0.100000001f));
      x_261_phi = x_198;
    } else {
      float x_216 = 0.0f;
      float x_258 = 0.0f;
      float x_259_phi = 0.0f;
      if ((x_180 < 90)) {
        const int x_204 = data[2];
        x_207 = (0.5f + (float(x_204) * 0.100000001f));
        x_260_phi = x_207;
      } else {
        if ((x_180 < 120)) {
          const int x_213 = data[3];
          x_216 = (0.5f + (float(x_213) * 0.100000001f));
          x_259_phi = x_216;
        } else {
          float x_229 = 0.0f;
          float x_257 = 0.0f;
          float x_258_phi = 0.0f;
          if ((x_180 < 150)) {
            discard;
          } else {
            float x_238 = 0.0f;
            float x_256 = 0.0f;
            float x_257_phi = 0.0f;
            if ((x_180 < 180)) {
              const int x_226 = data[5];
              x_229 = (0.5f + (float(x_226) * 0.100000001f));
              x_258_phi = x_229;
            } else {
              float x_247 = 0.0f;
              float x_255 = 0.0f;
              float x_256_phi = 0.0f;
              if ((x_180 < 210)) {
                const int x_235 = data[6];
                x_238 = (0.5f + (float(x_235) * 0.100000001f));
                x_257_phi = x_238;
              } else {
                if ((x_180 < 240)) {
                  const int x_244 = data[7];
                  x_247 = (0.5f + (float(x_244) * 0.100000001f));
                  x_256_phi = x_247;
                } else {
                  if ((x_180 < 270)) {
                  } else {
                    discard;
                  }
                  const int x_252 = data[8];
                  x_255 = (0.5f + (float(x_252) * 0.100000001f));
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
  const float x_262 = x_262_phi;
  x_GLF_color = float4(x_262, x_262, x_262, 1.0f);
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
