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
      int x_285 = 0;
      int x_286 = 0;
      int x_305 = 0;
      int x_306 = 0;
      int x_320 = 0;
      int x_324 = 0;
      int x_339 = 0;
      int x_338 = 0;
      int x_352 = 0;
      int x_351 = 0;
      int x_366 = 0;
      int x_365 = 0;
      int x_287_phi = 0;
      int x_307_phi = 0;
      int x_328_phi = 0;
      int x_340_phi = 0;
      int x_353_phi = 0;
      int x_367_phi = 0;
      float x_261 = v.tint_symbol_3.injectionSwitch.x;
      if ((1.0f >= x_261)) {
      } else {
        {
        }
        continue;
      }
      int x_266 = i;
      int x_267 = mid;
      int x_269 = j;
      int x_270 = to;
      if (((x_266 <= x_267) & (x_269 <= x_270))) {
      } else {
        break;
      }
      int x_274 = i;
      int x_276 = data[x_274];
      int x_277 = j;
      int x_279 = data[x_277];
      bool x_280 = (x_276 < x_279);
      if (x_280) {
        x_285 = k;
        x_287_phi = x_285;
      } else {
        x_286 = 0;
        x_287_phi = x_286;
      }
      int x_287 = x_287_phi;
      int x_288 = (x_287 + 1);
      if (x_280) {
        k = x_288;
        float x_293 = v.tint_symbol_3.injectionSwitch.x;
        if (!((1.0f <= x_293))) {
        } else {
          {
          }
          continue;
        }
      }
      float x_297 = v.tint_symbol_3.injectionSwitch.y;
      if ((x_297 >= 0.0f)) {
      } else {
        {
        }
        continue;
      }
      int x_300 = 0;
      if (x_280) {
        x_305 = i;
        x_307_phi = x_305;
      } else {
        x_306 = 0;
        x_307_phi = x_306;
      }
      int x_307 = x_307_phi;
      int x_309 = ((x_280) ? (x_307) : (x_300));
      if (x_280) {
        i = (x_309 + 1);
      }
      int x_315 = 0;
      if (x_280) {
        x_320 = data[x_309];
        float x_322 = v.tint_symbol_3.injectionSwitch.y;
        x_328_phi = x_320;
        if (!((0.0f <= x_322))) {
          {
          }
          continue;
        }
      } else {
        x_324 = 0;
        float x_326 = v.tint_symbol_3.injectionSwitch.y;
        x_328_phi = x_324;
        if (!((x_326 < 0.0f))) {
        } else {
          {
          }
          continue;
        }
      }
      int x_328 = x_328_phi;
      if (x_280) {
        temp[x_287] = ((x_280) ? (x_328) : (x_315));
      }
      if (x_280) {
        x_339 = 0;
        x_340_phi = x_339;
      } else {
        x_338 = k;
        x_340_phi = x_338;
      }
      int x_340 = x_340_phi;
      if (x_280) {
      } else {
        k = (x_340 + 1);
      }
      float x_345 = v.tint_symbol_3.injectionSwitch.x;
      if (!((1.0f <= x_345))) {
      } else {
        {
        }
        continue;
      }
      if (x_280) {
        x_352 = 0;
        x_353_phi = x_352;
      } else {
        x_351 = j;
        x_353_phi = x_351;
      }
      int x_353 = x_353_phi;
      int x_355 = 0;
      int x_357 = ((x_280) ? (x_355) : (x_353));
      if (x_280) {
      } else {
        j = (x_357 + 1);
      }
      if (x_280) {
        x_366 = 0;
        x_367_phi = x_366;
      } else {
        x_365 = data[x_357];
        x_367_phi = x_365;
      }
      int x_367 = x_367_phi;
      if (x_280) {
      } else {
        temp[x_340] = x_367;
      }
      {
      }
      continue;
    }
  }
  {
    while(true) {
      int x_376 = i;
      int x_378 = i;
      int x_379 = mid;
      if (((x_376 < 10) & (x_378 <= x_379))) {
      } else {
        break;
      }
      int x_383 = k;
      k = (x_383 + 1);
      int x_385 = i;
      i = (x_385 + 1);
      int x_388 = data[x_385];
      temp[x_383] = x_388;
      {
      }
      continue;
    }
  }
  int x_390 = f;
  i_1 = x_390;
  {
    while(true) {
      int x_395 = i_1;
      int x_396 = to;
      if ((x_395 <= x_396)) {
      } else {
        break;
      }
      int x_399 = i_1;
      int x_400 = i_1;
      int x_402 = temp[x_400];
      data[x_399] = x_402;
      {
        int x_404 = i_1;
        i_1 = (x_404 + 1);
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
      int x_411 = m;
      int x_412 = high;
      if ((x_411 <= x_412)) {
      } else {
        break;
      }
      int x_415 = low;
      i_2 = x_415;
      {
        while(true) {
          int x_420 = i_2;
          int x_421 = high;
          if ((x_420 < x_421)) {
          } else {
            break;
          }
          int x_424 = i_2;
          f_1 = x_424;
          int x_425 = i_2;
          int x_426 = m;
          mid_1 = ((x_425 + x_426) - 1);
          int x_429 = i_2;
          int x_430 = m;
          int x_434 = high;
          to_1 = min(((x_429 + (2 * x_430)) - 1), x_434);
          int x_436 = f_1;
          param = x_436;
          int x_437 = mid_1;
          param_1 = x_437;
          int x_438 = to_1;
          param_2 = x_438;
          merge_i1_i1_i1_(param, param_1, param_2);
          {
            int x_440 = m;
            int x_442 = i_2;
            i_2 = (x_442 + (2 * x_440));
          }
          continue;
        }
      }
      {
        int x_444 = m;
        m = (2 * x_444);
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
ERROR: 0:66: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:66: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
