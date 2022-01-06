SKIP: FAILED

#version 310 es
precision mediump float;

struct buf0 {
  vec2 injectionSwitch;
};

int data[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
int temp[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
layout (binding = 0) uniform buf0_1 {
  vec2 injectionSwitch;
} x_28;
vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void merge_i1_i1_i1_(inout int from, inout int mid, inout int to) {
  int k = 0;
  int i = 0;
  int j = 0;
  int i_1 = 0;
  int x_255 = from;
  k = x_255;
  int x_256 = from;
  i = x_256;
  int x_257 = mid;
  j = (x_257 + 1);
  while (true) {
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
    if ((1.0f >= 0.0f)) {
    } else {
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
    int x_274 = data[i];
    int x_277 = data[j];
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
      float x_291 = x_28.injectionSwitch.x;
      if (!((1.0f <= x_291))) {
      } else {
        continue;
      }
    }
    float x_295 = x_28.injectionSwitch.y;
    if ((x_295 >= 0.0f)) {
    } else {
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
    int x_307 = (x_278 ? x_305_phi : x_298);
    if (x_278) {
      i = (x_307 + 1);
    }
    int x_313 = 0;
    if (x_278) {
      x_318 = data[x_307];
      float x_320 = x_28.injectionSwitch.y;
      x_326_phi = x_318;
      if (!((0.0f <= x_320))) {
        continue;
      }
    } else {
      x_322 = 0;
      float x_324 = x_28.injectionSwitch.y;
      x_326_phi = x_322;
      if (!((x_324 < 0.0f))) {
      } else {
        continue;
      }
    }
    int x_326 = x_326_phi;
    if (x_278) {
      temp[x_285] = (x_278 ? x_326 : x_313);
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
    float x_343 = x_28.injectionSwitch.x;
    if (!((1.0f <= x_343))) {
    } else {
      continue;
    }
    if (x_278) {
      x_350 = 0;
      x_351_phi = x_350;
    } else {
      x_349 = j;
      x_351_phi = x_349;
    }
    int x_355 = (x_278 ? 0 : x_351_phi);
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
  }
  while (true) {
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
  }
  int x_388 = from;
  i_1 = x_388;
  while (true) {
    int x_393 = i_1;
    int x_394 = to;
    if ((x_393 <= x_394)) {
    } else {
      break;
    }
    int x_397 = i_1;
    int x_400 = temp[i_1];
    data[x_397] = x_400;
    {
      i_1 = (i_1 + 1);
    }
  }
  return;
}

void mergeSort_() {
  int low = 0;
  int high = 0;
  int m = 0;
  int i_2 = 0;
  int from_1 = 0;
  int mid_1 = 0;
  int to_1 = 0;
  int param = 0;
  int param_1 = 0;
  int param_2 = 0;
  low = 0;
  high = 9;
  m = 1;
  {
    for(; (m <= high); m = (2 * m)) {
      i_2 = low;
      {
        for(; (i_2 < high); i_2 = (i_2 + (2 * m))) {
          from_1 = i_2;
          mid_1 = ((i_2 + m) - 1);
          to_1 = min(((i_2 + (2 * m)) - 1), high);
          param = from_1;
          param_1 = mid_1;
          param_2 = to_1;
          merge_i1_i1_i1_(param, param_1, param_2);
        }
      }
    }
  }
  return;
}

void main_1() {
  int i_3 = 0;
  int j_1 = 0;
  float grey = 0.0f;
  float x_88 = x_28.injectionSwitch.x;
  i_3 = int(x_88);
  while (true) {
    switch(i_3) {
      case 9: {
        data[i_3] = -5;
        break;
      }
      case 8: {
        data[i_3] = -4;
        break;
      }
      case 7: {
        data[i_3] = -3;
        break;
      }
      case 6: {
        data[i_3] = -2;
        break;
      }
      case 5: {
        data[i_3] = -1;
        break;
      }
      case 4: {
        data[i_3] = 0;
        break;
      }
      case 3: {
        data[i_3] = 1;
        break;
      }
      case 2: {
        data[i_3] = 2;
        break;
      }
      case 1: {
        data[i_3] = 3;
        break;
      }
      case 0: {
        data[i_3] = 4;
        break;
      }
      default: {
        break;
      }
    }
    i_3 = (i_3 + 1);
    {
      if ((i_3 < 10)) {
      } else {
        break;
      }
    }
  }
  j_1 = 0;
  {
    for(; (j_1 < 10); j_1 = (j_1 + 1)) {
      int x_137 = j_1;
      int x_140 = data[j_1];
      temp[x_137] = x_140;
    }
  }
  mergeSort_();
  float x_146 = tint_symbol.y;
  if ((int(x_146) < 30)) {
    int x_153 = data[0];
    grey = (0.5f + (float(x_153) / 10.0f));
  } else {
    float x_158 = tint_symbol.y;
    if ((int(x_158) < 60)) {
      int x_165 = data[1];
      grey = (0.5f + (float(x_165) / 10.0f));
    } else {
      float x_170 = tint_symbol.y;
      if ((int(x_170) < 90)) {
        int x_177 = data[2];
        grey = (0.5f + (float(x_177) / 10.0f));
      } else {
        float x_182 = tint_symbol.y;
        if ((int(x_182) < 120)) {
          int x_189 = data[3];
          grey = (0.5f + (float(x_189) / 10.0f));
        } else {
          float x_194 = tint_symbol.y;
          if ((int(x_194) < 150)) {
            discard;
          } else {
            float x_201 = tint_symbol.y;
            if ((int(x_201) < 180)) {
              int x_208 = data[5];
              grey = (0.5f + (float(x_208) / 10.0f));
            } else {
              float x_213 = tint_symbol.y;
              if ((int(x_213) < 210)) {
                int x_220 = data[6];
                grey = (0.5f + (float(x_220) / 10.0f));
              } else {
                float x_225 = tint_symbol.y;
                if ((int(x_225) < 240)) {
                  int x_232 = data[7];
                  grey = (0.5f + (float(x_232) / 10.0f));
                } else {
                  float x_237 = tint_symbol.y;
                  if ((int(x_237) < 270)) {
                    int x_244 = data[8];
                    grey = (0.5f + (float(x_244) / 10.0f));
                  } else {
                    discard;
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
  x_GLF_color = vec4(x_249.x, x_249.y, x_249.z, 1.0f);
  return;
}

struct main_out {
  vec4 x_GLF_color_1;
};
struct tint_symbol_4 {
  vec4 tint_symbol_2;
};
struct tint_symbol_5 {
  vec4 x_GLF_color_1;
};

main_out tint_symbol_1_inner(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  main_out tint_symbol_6 = main_out(x_GLF_color);
  return tint_symbol_6;
}

tint_symbol_5 tint_symbol_1(tint_symbol_4 tint_symbol_3) {
  main_out inner_result = tint_symbol_1_inner(tint_symbol_3.tint_symbol_2);
  tint_symbol_5 wrapper_result = tint_symbol_5(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
out vec4 x_GLF_color_1;
void main() {
  tint_symbol_4 inputs;
  inputs.tint_symbol_2 = gl_FragCoord;
  tint_symbol_5 outputs;
  outputs = tint_symbol_1(inputs);
  x_GLF_color_1 = outputs.x_GLF_color_1;
}


Error parsing GLSL shader:
ERROR: 0:54: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:54: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



