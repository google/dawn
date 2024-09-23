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
  int x_303 = f;
  k = x_303;
  int x_304 = f;
  i = x_304;
  int x_305 = mid;
  j = (x_305 + 1);
  {
    while(true) {
      int x_311 = i;
      int x_312 = mid;
      int x_314 = j;
      int x_315 = to;
      if (((x_311 <= x_312) & (x_314 <= x_315))) {
      } else {
        break;
      }
      int x_319 = i;
      int x_321 = data[x_319];
      int x_322 = j;
      int x_324 = data[x_322];
      if ((x_321 < x_324)) {
        int x_329 = k;
        k = (x_329 + 1);
        int x_331 = i;
        i = (x_331 + 1);
        int x_334 = data[x_331];
        temp[x_329] = x_334;
      } else {
        int x_336 = k;
        k = (x_336 + 1);
        int x_338 = j;
        j = (x_338 + 1);
        int x_341 = data[x_338];
        temp[x_336] = x_341;
      }
      {
      }
      continue;
    }
  }
  {
    while(true) {
      int x_347 = i;
      int x_349 = i;
      int x_350 = mid;
      if (((x_347 < 10) & (x_349 <= x_350))) {
      } else {
        break;
      }
      int x_354 = k;
      k = (x_354 + 1);
      int x_356 = i;
      i = (x_356 + 1);
      int x_359 = data[x_356];
      temp[x_354] = x_359;
      {
      }
      continue;
    }
  }
  int x_361 = f;
  i_1 = x_361;
  {
    while(true) {
      int x_366 = i_1;
      int x_367 = to;
      if ((x_366 <= x_367)) {
      } else {
        break;
      }
      int x_370 = i_1;
      int x_371 = i_1;
      int x_373 = temp[x_371];
      data[x_370] = x_373;
      {
        int x_375 = i_1;
        i_1 = (x_375 + 1);
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
          int x_173[10] = data;
          data = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
          data = x_173;
          x_89 = ((x_170 + x_171) - 1);
          int x_175 = x_91;
          int x_176 = x_92;
          int x_180 = x_93;
          x_88 = min(((x_175 + (2 * x_176)) - 1), x_180);
          int x_182 = x_90;
          x_87 = x_182;
          int x_183 = x_89;
          x_86 = x_183;
          int x_184 = x_88;
          x_85 = x_184;
          merge_i1_i1_i1_(x_87, x_86, x_85);
          {
            int x_186 = x_92;
            int x_188 = x_91;
            x_91 = (x_188 + (2 * x_186));
          }
          continue;
        }
      }
      {
        int x_190 = x_92;
        x_92 = (2 * x_190);
      }
      continue;
    }
  }
  float x_194 = tint_symbol.y;
  if ((tint_f32_to_i32(x_194) < 30)) {
    int x_201 = data[0];
    grey = (0.5f + (float(x_201) / 10.0f));
  } else {
    float x_206 = tint_symbol.y;
    if ((tint_f32_to_i32(x_206) < 60)) {
      int x_213 = data[1];
      grey = (0.5f + (float(x_213) / 10.0f));
    } else {
      float x_218 = tint_symbol.y;
      if ((tint_f32_to_i32(x_218) < 90)) {
        int x_225 = data[2];
        grey = (0.5f + (float(x_225) / 10.0f));
      } else {
        float x_230 = tint_symbol.y;
        if ((tint_f32_to_i32(x_230) < 120)) {
          int x_237 = data[3];
          grey = (0.5f + (float(x_237) / 10.0f));
        } else {
          float x_242 = tint_symbol.y;
          if ((tint_f32_to_i32(x_242) < 150)) {
            continue_execution = false;
          } else {
            float x_249 = tint_symbol.y;
            if ((tint_f32_to_i32(x_249) < 180)) {
              int x_256 = data[5];
              grey = (0.5f + (float(x_256) / 10.0f));
            } else {
              float x_261 = tint_symbol.y;
              if ((tint_f32_to_i32(x_261) < 210)) {
                int x_268 = data[6];
                grey = (0.5f + (float(x_268) / 10.0f));
              } else {
                float x_273 = tint_symbol.y;
                if ((tint_f32_to_i32(x_273) < 240)) {
                  int x_280 = data[7];
                  grey = (0.5f + (float(x_280) / 10.0f));
                } else {
                  float x_285 = tint_symbol.y;
                  if ((tint_f32_to_i32(x_285) < 270)) {
                    int x_292 = data[8];
                    grey = (0.5f + (float(x_292) / 10.0f));
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
  float x_296 = grey;
  vec3 x_297 = vec3(x_296, x_296, x_296);
  x_GLF_color = vec4(x_297[0u], x_297[1u], x_297[2u], 1.0f);
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
