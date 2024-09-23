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
vec4 tint_symbol = vec4(0.0f);
layout(binding = 0, std140)
uniform tint_symbol_4_1_ubo {
  buf0 tint_symbol_3;
} v;
vec4 x_GLF_color = vec4(0.0f);
bool continue_execution = true;
layout(location = 0) out vec4 tint_symbol_1_loc0_Output;
void merge_i1_i1_i1_(inout int f, inout int mid, inout int to) {
  int k = 0;
  int i = 0;
  int j = 0;
  int i_1 = 0;
  int x_260 = f;
  k = x_260;
  int x_261 = f;
  i = x_261;
  int x_262 = mid;
  j = (x_262 + 1);
  {
    while(true) {
      int x_268 = i;
      int x_269 = mid;
      int x_271 = j;
      int x_272 = to;
      if (((x_268 <= x_269) & (x_271 <= x_272))) {
      } else {
        break;
      }
      int x_276 = i;
      int x_278 = data[x_276];
      int x_279 = j;
      int x_281 = data[x_279];
      if ((x_278 < x_281)) {
        int x_286 = k;
        k = (x_286 + 1);
        int x_288 = i;
        i = (x_288 + 1);
        int x_291 = data[x_288];
        temp[x_286] = x_291;
      } else {
        int x_293 = k;
        k = (x_293 + 1);
        int x_295 = j;
        j = (x_295 + 1);
        int x_298 = data[x_295];
        temp[x_293] = x_298;
      }
      {
      }
      continue;
    }
  }
  {
    while(true) {
      int x_304 = i;
      int x_306 = i;
      int x_307 = mid;
      if (((x_304 < 10) & (x_306 <= x_307))) {
      } else {
        break;
      }
      int x_311 = k;
      k = (x_311 + 1);
      int x_313 = i;
      i = (x_313 + 1);
      int x_316 = data[x_313];
      temp[x_311] = x_316;
      {
      }
      continue;
    }
  }
  int x_318 = f;
  i_1 = x_318;
  {
    while(true) {
      int x_323 = i_1;
      int x_324 = to;
      if ((x_323 <= x_324)) {
      } else {
        break;
      }
      int x_327 = i_1;
      int x_328 = i_1;
      int x_330 = temp[x_328];
      data[x_327] = x_330;
      {
        int x_332 = i_1;
        i_1 = (x_332 + 1);
      }
      continue;
    }
  }
}
int tint_div_i32(int lhs, int rhs) {
  return (lhs / ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs)));
}
int func_i1_i1_(inout int m, inout int high) {
  int x = 0;
  int x_335 = 0;
  int x_336 = 0;
  float x_338 = tint_symbol.x;
  if ((x_338 >= 0.0f)) {
    if (false) {
      int x_346 = high;
      x_336 = (x_346 << (0u & 31u));
    } else {
      x_336 = 4;
    }
    int x_348 = x_336;
    x_335 = (1 << (uint(x_348) & 31u));
  } else {
    x_335 = 1;
  }
  int x_350 = x_335;
  x = x_350;
  int x_351 = x;
  x = (x_351 >> (4u & 31u));
  int x_353 = m;
  int x_355 = m;
  int x_357 = m;
  int x_359 = x;
  int v_1 = tint_div_i32((2 * x_357), x_359);
  return min(max((2 * x_353), (2 * x_355)), v_1);
}
void mergeSort_() {
  int low = 0;
  int high_1 = 0;
  int m_1 = 0;
  int i_2 = 0;
  int f_1 = 0;
  int mid_1 = 0;
  int to_1 = 0;
  int param = 0;
  int param_1 = 0;
  int param_2 = 0;
  int param_3 = 0;
  int param_4 = 0;
  low = 0;
  high_1 = 9;
  m_1 = 1;
  {
    while(true) {
      int x_367 = m_1;
      int x_368 = high_1;
      if ((x_367 <= x_368)) {
      } else {
        break;
      }
      int x_371 = low;
      i_2 = x_371;
      {
        while(true) {
          int x_376 = i_2;
          int x_377 = high_1;
          if ((x_376 < x_377)) {
          } else {
            break;
          }
          int x_380 = i_2;
          f_1 = x_380;
          int x_381 = i_2;
          int x_382 = m_1;
          mid_1 = ((x_381 + x_382) - 1);
          int x_385 = i_2;
          int x_386 = m_1;
          int x_390 = high_1;
          to_1 = min(((x_385 + (2 * x_386)) - 1), x_390);
          int x_392 = f_1;
          param = x_392;
          int x_393 = mid_1;
          param_1 = x_393;
          int x_394 = to_1;
          param_2 = x_394;
          merge_i1_i1_i1_(param, param_1, param_2);
          {
            int x_396 = m_1;
            param_3 = x_396;
            int x_397 = high_1;
            param_4 = x_397;
            int x_398 = func_i1_i1_(param_3, param_4);
            int x_399 = i_2;
            i_2 = (x_399 + x_398);
          }
          continue;
        }
      }
      {
        int x_401 = m_1;
        m_1 = (2 * x_401);
      }
      continue;
    }
  }
}
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : ((-2147483647 - 1)))) : (2147483647));
}
void main_1() {
  int i_3 = 0;
  int j_1 = 0;
  float grey = 0.0f;
  float x_93 = v.tint_symbol_3.injectionSwitch.x;
  i_3 = tint_f32_to_i32(x_93);
  {
    while(true) {
      int x_99 = i_3;
      switch(x_99) {
        case 9:
        {
          int x_129 = i_3;
          data[x_129] = -5;
          break;
        }
        case 8:
        {
          int x_127 = i_3;
          data[x_127] = -4;
          break;
        }
        case 7:
        {
          int x_125 = i_3;
          data[x_125] = -3;
          break;
        }
        case 6:
        {
          int x_123 = i_3;
          data[x_123] = -2;
          break;
        }
        case 5:
        {
          int x_121 = i_3;
          data[x_121] = -1;
          break;
        }
        case 4:
        {
          int x_119 = i_3;
          data[x_119] = 0;
          break;
        }
        case 3:
        {
          int x_117 = i_3;
          data[x_117] = 1;
          break;
        }
        case 2:
        {
          int x_115 = i_3;
          data[x_115] = 2;
          break;
        }
        case 1:
        {
          int x_113 = i_3;
          data[x_113] = 3;
          break;
        }
        case 0:
        {
          int x_111 = i_3;
          data[x_111] = 4;
          break;
        }
        default:
        {
          break;
        }
      }
      int x_131 = i_3;
      i_3 = (x_131 + 1);
      {
        int x_133 = i_3;
        if (!((x_133 < 10))) { break; }
      }
      continue;
    }
  }
  j_1 = 0;
  {
    while(true) {
      int x_139 = j_1;
      if ((x_139 < 10)) {
      } else {
        break;
      }
      int x_142 = j_1;
      int x_143 = j_1;
      int x_145 = data[x_143];
      temp[x_142] = x_145;
      {
        int x_147 = j_1;
        j_1 = (x_147 + 1);
      }
      continue;
    }
  }
  mergeSort_();
  float x_151 = tint_symbol.y;
  if ((tint_f32_to_i32(x_151) < 30)) {
    int x_158 = data[0];
    grey = (0.5f + (float(x_158) / 10.0f));
  } else {
    float x_163 = tint_symbol.y;
    if ((tint_f32_to_i32(x_163) < 60)) {
      int x_170 = data[1];
      grey = (0.5f + (float(x_170) / 10.0f));
    } else {
      float x_175 = tint_symbol.y;
      if ((tint_f32_to_i32(x_175) < 90)) {
        int x_182 = data[2];
        grey = (0.5f + (float(x_182) / 10.0f));
      } else {
        float x_187 = tint_symbol.y;
        if ((tint_f32_to_i32(x_187) < 120)) {
          int x_194 = data[3];
          grey = (0.5f + (float(x_194) / 10.0f));
        } else {
          float x_199 = tint_symbol.y;
          if ((tint_f32_to_i32(x_199) < 150)) {
            continue_execution = false;
          } else {
            float x_206 = tint_symbol.y;
            if ((tint_f32_to_i32(x_206) < 180)) {
              int x_213 = data[5];
              grey = (0.5f + (float(x_213) / 10.0f));
            } else {
              float x_218 = tint_symbol.y;
              if ((tint_f32_to_i32(x_218) < 210)) {
                int x_225 = data[6];
                grey = (0.5f + (float(x_225) / 10.0f));
              } else {
                float x_230 = tint_symbol.y;
                if ((tint_f32_to_i32(x_230) < 240)) {
                  int x_237 = data[7];
                  grey = (0.5f + (float(x_237) / 10.0f));
                } else {
                  float x_242 = tint_symbol.y;
                  if ((tint_f32_to_i32(x_242) < 270)) {
                    int x_249 = data[8];
                    grey = (0.5f + (float(x_249) / 10.0f));
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
  float x_253 = grey;
  vec3 x_254 = vec3(x_253, x_253, x_253);
  x_GLF_color = vec4(x_254[0u], x_254[1u], x_254[2u], 1.0f);
}
main_out tint_symbol_1_inner(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  main_out v_2 = main_out(x_GLF_color);
  if (!(continue_execution)) {
    discard;
  }
  return v_2;
}
void main() {
  tint_symbol_1_loc0_Output = tint_symbol_1_inner(gl_FragCoord).x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:41: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:41: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
