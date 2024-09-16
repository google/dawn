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

int data[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
int temp[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
layout(binding = 0, std140)
uniform tint_symbol_4_1_ubo {
  buf0 tint_symbol_3;
} v;
vec4 tint_symbol = vec4(0.0f);
vec4 x_GLF_color = vec4(0.0f);
bool continue_execution = true;
layout(location = 0) out vec4 tint_symbol_1_loc0_Output;
void merge_i1_i1_i1_(inout int f, inout int mid, inout int to) {
  int k = 0;
  int i = 0;
  int j = 0;
  int i_1 = 0;
  int x_302 = f;
  k = x_302;
  int x_303 = f;
  i = x_303;
  int x_304 = mid;
  j = (x_304 + 1);
  {
    while(true) {
      int x_310 = i;
      int x_311 = mid;
      int x_313 = j;
      int x_314 = to;
      if (((x_310 <= x_311) & (x_313 <= x_314))) {
      } else {
        break;
      }
      int x_318 = i;
      int x_320 = data[x_318];
      int x_321 = j;
      int x_323 = data[x_321];
      if ((x_320 < x_323)) {
        int x_328 = k;
        k = (x_328 + 1);
        int x_330 = i;
        i = (x_330 + 1);
        int x_333 = data[x_330];
        temp[x_328] = x_333;
      } else {
        int x_335 = k;
        k = (x_335 + 1);
        int x_337 = j;
        j = (x_337 + 1);
        int x_340 = data[x_337];
        temp[x_335] = x_340;
      }
      {
      }
      continue;
    }
  }
  {
    while(true) {
      int x_346 = i;
      int x_348 = i;
      int x_349 = mid;
      if (((x_346 < 10) & (x_348 <= x_349))) {
      } else {
        break;
      }
      int x_353 = k;
      k = (x_353 + 1);
      int x_355 = i;
      i = (x_355 + 1);
      int x_358 = data[x_355];
      temp[x_353] = x_358;
      {
      }
      continue;
    }
  }
  int x_360 = f;
  i_1 = x_360;
  {
    while(true) {
      int x_365 = i_1;
      int x_366 = to;
      if ((x_365 <= x_366)) {
      } else {
        break;
      }
      int x_369 = i_1;
      int x_370 = i_1;
      int x_372 = temp[x_370];
      data[x_369] = x_372;
      {
        int x_374 = i_1;
        i_1 = (x_374 + 1);
      }
      continue;
    }
  }
}
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : ((-2147483647 - 1)))) : (2147483647));
}
void main_1() {
  int x_85 = 0;
  int x_86 = 0;
  int x_87 = 0;
  int x_88 = 0;
  int x_89 = 0;
  int x_90 = 0;
  int x_91 = 0;
  int x_92 = 0;
  int x_93 = 0;
  int x_94 = 0;
  int i_3 = 0;
  int j_1 = 0;
  float grey = 0.0f;
  float x_96 = v.tint_symbol_3.injectionSwitch.x;
  i_3 = tint_f32_to_i32(x_96);
  {
    while(true) {
      int x_102 = i_3;
      switch(x_102) {
        case 9:
        {
          int x_132 = i_3;
          data[x_132] = -5;
          break;
        }
        case 8:
        {
          int x_130 = i_3;
          data[x_130] = -4;
          break;
        }
        case 7:
        {
          int x_128 = i_3;
          data[x_128] = -3;
          break;
        }
        case 6:
        {
          int x_126 = i_3;
          data[x_126] = -2;
          break;
        }
        case 5:
        {
          int x_124 = i_3;
          data[x_124] = -1;
          break;
        }
        case 4:
        {
          int x_122 = i_3;
          data[x_122] = 0;
          break;
        }
        case 3:
        {
          int x_120 = i_3;
          data[x_120] = 1;
          break;
        }
        case 2:
        {
          int x_118 = i_3;
          data[x_118] = 2;
          break;
        }
        case 1:
        {
          int x_116 = i_3;
          data[x_116] = 3;
          break;
        }
        case 0:
        {
          int x_114 = i_3;
          data[x_114] = 4;
          break;
        }
        default:
        {
          break;
        }
      }
      int x_134 = i_3;
      i_3 = (x_134 + 1);
      {
        int x_136 = i_3;
        if (!((x_136 < 10))) { break; }
      }
      continue;
    }
  }
  j_1 = 0;
  {
    while(true) {
      int x_142 = j_1;
      if ((x_142 < 10)) {
      } else {
        break;
      }
      int x_145 = j_1;
      int x_146 = j_1;
      int x_148 = data[x_146];
      temp[x_145] = x_148;
      {
        int x_150 = j_1;
        j_1 = (x_150 + 1);
      }
      continue;
    }
  }
  x_94 = 0;
  x_93 = 9;
  x_92 = 1;
  {
    while(true) {
      int x_156 = x_92;
      int x_157 = x_93;
      if ((x_156 <= x_157)) {
      } else {
        break;
      }
      int x_160 = x_94;
      x_91 = x_160;
      {
        while(true) {
          int x_165 = x_91;
          int x_166 = x_93;
          if ((x_165 < x_166)) {
          } else {
            break;
          }
          int x_169 = x_91;
          x_90 = x_169;
          int x_170 = x_91;
          int x_171 = x_92;
          x_89 = ((x_170 + x_171) - 1);
          int x_174 = x_91;
          int x_175 = x_92;
          int x_179 = x_93;
          x_88 = min(((x_174 + (2 * x_175)) - 1), x_179);
          int x_181 = x_90;
          x_87 = x_181;
          int x_182 = x_89;
          x_86 = x_182;
          int x_183 = x_88;
          x_85 = x_183;
          merge_i1_i1_i1_(x_87, x_86, x_85);
          {
            int x_185 = x_92;
            int x_187 = x_91;
            x_91 = (x_187 + (2 * x_185));
          }
          continue;
        }
      }
      {
        int x_189 = x_92;
        x_92 = (2 * x_189);
      }
      continue;
    }
  }
  float x_193 = tint_symbol.y;
  if ((tint_f32_to_i32(x_193) < 30)) {
    int x_200 = data[0];
    grey = (0.5f + (float(x_200) / 10.0f));
  } else {
    float x_205 = tint_symbol.y;
    if ((tint_f32_to_i32(x_205) < 60)) {
      int x_212 = data[1];
      grey = (0.5f + (float(x_212) / 10.0f));
    } else {
      float x_217 = tint_symbol.y;
      if ((tint_f32_to_i32(x_217) < 90)) {
        int x_224 = data[2];
        grey = (0.5f + (float(x_224) / 10.0f));
      } else {
        float x_229 = tint_symbol.y;
        if ((tint_f32_to_i32(x_229) < 120)) {
          int x_236 = data[3];
          grey = (0.5f + (float(x_236) / 10.0f));
        } else {
          float x_241 = tint_symbol.y;
          if ((tint_f32_to_i32(x_241) < 150)) {
            continue_execution = false;
          } else {
            float x_248 = tint_symbol.y;
            if ((tint_f32_to_i32(x_248) < 180)) {
              int x_255 = data[5];
              grey = (0.5f + (float(x_255) / 10.0f));
            } else {
              float x_260 = tint_symbol.y;
              if ((tint_f32_to_i32(x_260) < 210)) {
                int x_267 = data[6];
                grey = (0.5f + (float(x_267) / 10.0f));
              } else {
                float x_272 = tint_symbol.y;
                if ((tint_f32_to_i32(x_272) < 240)) {
                  int x_279 = data[7];
                  grey = (0.5f + (float(x_279) / 10.0f));
                } else {
                  float x_284 = tint_symbol.y;
                  if ((tint_f32_to_i32(x_284) < 270)) {
                    int x_291 = data[8];
                    grey = (0.5f + (float(x_291) / 10.0f));
                  } else {
                    continue_execution = false;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  float x_295 = grey;
  vec3 x_296 = vec3(x_295, x_295, x_295);
  x_GLF_color = vec4(x_296[0u], x_296[1u], x_296[2u], 1.0f);
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
ERROR: 0:41: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:41: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
