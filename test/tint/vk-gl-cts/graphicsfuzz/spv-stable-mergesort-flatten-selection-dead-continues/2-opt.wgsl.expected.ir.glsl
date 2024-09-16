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
  int x_255 = f;
  k = x_255;
  int x_256 = f;
  i = x_256;
  int x_257 = mid;
  j = (x_257 + 1);
  {
    while(true) {
      int x_283 = 0;
      int x_284 = 0;
      int x_303 = 0;
      int x_304 = 0;
      int x_318 = 0;
      int x_322 = 0;
      int x_337 = 0;
      int x_336 = 0;
      int x_350 = 0;
      int x_349 = 0;
      int x_364 = 0;
      int x_363 = 0;
      int x_285_phi = 0;
      int x_305_phi = 0;
      int x_326_phi = 0;
      int x_338_phi = 0;
      int x_351_phi = 0;
      int x_365_phi = 0;
      if (true) {
      } else {
        {
        }
        continue;
      }
      int x_264 = i;
      int x_265 = mid;
      int x_267 = j;
      int x_268 = to;
      if (((x_264 <= x_265) & (x_267 <= x_268))) {
      } else {
        break;
      }
      int x_272 = i;
      int x_274 = data[x_272];
      int x_275 = j;
      int x_277 = data[x_275];
      bool x_278 = (x_274 < x_277);
      if (x_278) {
        x_283 = k;
        x_285_phi = x_283;
      } else {
        x_284 = 0;
        x_285_phi = x_284;
      }
      int x_285 = x_285_phi;
      int x_286 = (x_285 + 1);
      if (x_278) {
        k = x_286;
        float x_291 = v.tint_symbol_3.injectionSwitch.x;
        if (!((1.0f <= x_291))) {
        } else {
          {
          }
          continue;
        }
      }
      float x_295 = v.tint_symbol_3.injectionSwitch.y;
      if ((x_295 >= 0.0f)) {
      } else {
        {
        }
        continue;
      }
      int x_298 = 0;
      if (x_278) {
        x_303 = i;
        x_305_phi = x_303;
      } else {
        x_304 = 0;
        x_305_phi = x_304;
      }
      int x_305 = x_305_phi;
      int x_307 = ((x_278) ? (x_305) : (x_298));
      if (x_278) {
        i = (x_307 + 1);
      }
      int x_313 = 0;
      if (x_278) {
        x_318 = data[x_307];
        float x_320 = v.tint_symbol_3.injectionSwitch.y;
        x_326_phi = x_318;
        if (!((0.0f <= x_320))) {
          {
          }
          continue;
        }
      } else {
        x_322 = 0;
        float x_324 = v.tint_symbol_3.injectionSwitch.y;
        x_326_phi = x_322;
        if (!((x_324 < 0.0f))) {
        } else {
          {
          }
          continue;
        }
      }
      int x_326 = x_326_phi;
      if (x_278) {
        temp[x_285] = ((x_278) ? (x_326) : (x_313));
      }
      if (x_278) {
        x_337 = 0;
        x_338_phi = x_337;
      } else {
        x_336 = k;
        x_338_phi = x_336;
      }
      int x_338 = x_338_phi;
      if (x_278) {
      } else {
        k = (x_338 + 1);
      }
      float x_343 = v.tint_symbol_3.injectionSwitch.x;
      if (!((1.0f <= x_343))) {
      } else {
        {
        }
        continue;
      }
      if (x_278) {
        x_350 = 0;
        x_351_phi = x_350;
      } else {
        x_349 = j;
        x_351_phi = x_349;
      }
      int x_351 = x_351_phi;
      int x_353 = 0;
      int x_355 = ((x_278) ? (x_353) : (x_351));
      if (x_278) {
      } else {
        j = (x_355 + 1);
      }
      if (x_278) {
        x_364 = 0;
        x_365_phi = x_364;
      } else {
        x_363 = data[x_355];
        x_365_phi = x_363;
      }
      int x_365 = x_365_phi;
      if (x_278) {
      } else {
        temp[x_338] = x_365;
      }
      {
      }
      continue;
    }
  }
  {
    while(true) {
      int x_374 = i;
      int x_376 = i;
      int x_377 = mid;
      if (((x_374 < 10) & (x_376 <= x_377))) {
      } else {
        break;
      }
      int x_381 = k;
      k = (x_381 + 1);
      int x_383 = i;
      i = (x_383 + 1);
      int x_386 = data[x_383];
      temp[x_381] = x_386;
      {
      }
      continue;
    }
  }
  int x_388 = f;
  i_1 = x_388;
  {
    while(true) {
      int x_393 = i_1;
      int x_394 = to;
      if ((x_393 <= x_394)) {
      } else {
        break;
      }
      int x_397 = i_1;
      int x_398 = i_1;
      int x_400 = temp[x_398];
      data[x_397] = x_400;
      {
        int x_402 = i_1;
        i_1 = (x_402 + 1);
      }
      continue;
    }
  }
}
void mergeSort_() {
  int low = 0;
  int high = 0;
  int m = 0;
  int i_2 = 0;
  int f_1 = 0;
  int mid_1 = 0;
  int to_1 = 0;
  int param = 0;
  int param_1 = 0;
  int param_2 = 0;
  low = 0;
  high = 9;
  m = 1;
  {
    while(true) {
      int x_409 = m;
      int x_410 = high;
      if ((x_409 <= x_410)) {
      } else {
        break;
      }
      int x_413 = low;
      i_2 = x_413;
      {
        while(true) {
          int x_418 = i_2;
          int x_419 = high;
          if ((x_418 < x_419)) {
          } else {
            break;
          }
          int x_422 = i_2;
          f_1 = x_422;
          int x_423 = i_2;
          int x_424 = m;
          mid_1 = ((x_423 + x_424) - 1);
          int x_427 = i_2;
          int x_428 = m;
          int x_432 = high;
          to_1 = min(((x_427 + (2 * x_428)) - 1), x_432);
          int x_434 = f_1;
          param = x_434;
          int x_435 = mid_1;
          param_1 = x_435;
          int x_436 = to_1;
          param_2 = x_436;
          merge_i1_i1_i1_(param, param_1, param_2);
          {
            int x_438 = m;
            int x_440 = i_2;
            i_2 = (x_440 + (2 * x_438));
          }
          continue;
        }
      }
      {
        int x_442 = m;
        m = (2 * x_442);
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
  float x_88 = v.tint_symbol_3.injectionSwitch.x;
  i_3 = tint_f32_to_i32(x_88);
  {
    while(true) {
      int x_94 = i_3;
      switch(x_94) {
        case 9:
        {
          int x_124 = i_3;
          data[x_124] = -5;
          break;
        }
        case 8:
        {
          int x_122 = i_3;
          data[x_122] = -4;
          break;
        }
        case 7:
        {
          int x_120 = i_3;
          data[x_120] = -3;
          break;
        }
        case 6:
        {
          int x_118 = i_3;
          data[x_118] = -2;
          break;
        }
        case 5:
        {
          int x_116 = i_3;
          data[x_116] = -1;
          break;
        }
        case 4:
        {
          int x_114 = i_3;
          data[x_114] = 0;
          break;
        }
        case 3:
        {
          int x_112 = i_3;
          data[x_112] = 1;
          break;
        }
        case 2:
        {
          int x_110 = i_3;
          data[x_110] = 2;
          break;
        }
        case 1:
        {
          int x_108 = i_3;
          data[x_108] = 3;
          break;
        }
        case 0:
        {
          int x_106 = i_3;
          data[x_106] = 4;
          break;
        }
        default:
        {
          break;
        }
      }
      int x_126 = i_3;
      i_3 = (x_126 + 1);
      {
        int x_128 = i_3;
        if (!((x_128 < 10))) { break; }
      }
      continue;
    }
  }
  j_1 = 0;
  {
    while(true) {
      int x_134 = j_1;
      if ((x_134 < 10)) {
      } else {
        break;
      }
      int x_137 = j_1;
      int x_138 = j_1;
      int x_140 = data[x_138];
      temp[x_137] = x_140;
      {
        int x_142 = j_1;
        j_1 = (x_142 + 1);
      }
      continue;
    }
  }
  mergeSort_();
  float x_146 = tint_symbol.y;
  if ((tint_f32_to_i32(x_146) < 30)) {
    int x_153 = data[0];
    grey = (0.5f + (float(x_153) / 10.0f));
  } else {
    float x_158 = tint_symbol.y;
    if ((tint_f32_to_i32(x_158) < 60)) {
      int x_165 = data[1];
      grey = (0.5f + (float(x_165) / 10.0f));
    } else {
      float x_170 = tint_symbol.y;
      if ((tint_f32_to_i32(x_170) < 90)) {
        int x_177 = data[2];
        grey = (0.5f + (float(x_177) / 10.0f));
      } else {
        float x_182 = tint_symbol.y;
        if ((tint_f32_to_i32(x_182) < 120)) {
          int x_189 = data[3];
          grey = (0.5f + (float(x_189) / 10.0f));
        } else {
          float x_194 = tint_symbol.y;
          if ((tint_f32_to_i32(x_194) < 150)) {
            continue_execution = false;
          } else {
            float x_201 = tint_symbol.y;
            if ((tint_f32_to_i32(x_201) < 180)) {
              int x_208 = data[5];
              grey = (0.5f + (float(x_208) / 10.0f));
            } else {
              float x_213 = tint_symbol.y;
              if ((tint_f32_to_i32(x_213) < 210)) {
                int x_220 = data[6];
                grey = (0.5f + (float(x_220) / 10.0f));
              } else {
                float x_225 = tint_symbol.y;
                if ((tint_f32_to_i32(x_225) < 240)) {
                  int x_232 = data[7];
                  grey = (0.5f + (float(x_232) / 10.0f));
                } else {
                  float x_237 = tint_symbol.y;
                  if ((tint_f32_to_i32(x_237) < 270)) {
                    int x_244 = data[8];
                    grey = (0.5f + (float(x_244) / 10.0f));
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
  float x_248 = grey;
  vec3 x_249 = vec3(x_248, x_248, x_248);
  x_GLF_color = vec4(x_249[0u], x_249[1u], x_249[2u], 1.0f);
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
ERROR: 0:65: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:65: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
