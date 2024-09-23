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
  int x_256 = f;
  k = x_256;
  int x_257 = f;
  i = x_257;
  int x_258 = mid;
  j = (x_258 + 1);
  {
    while(true) {
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
      if ((x_274 < x_277)) {
        int x_282 = k;
        k = (x_282 + 1);
        int x_284 = i;
        i = (x_284 + 1);
        int x_287 = data[x_284];
        temp[x_282] = x_287;
      } else {
        int x_289 = k;
        k = (x_289 + 1);
        int x_291 = j;
        j = (x_291 + 1);
        int x_294 = data[x_291];
        temp[x_289] = x_294;
      }
      {
      }
      continue;
    }
  }
  {
    while(true) {
      if (true) {
      } else {
        {
        }
        continue;
      }
      int x_301 = i;
      int x_303 = i;
      int x_304 = mid;
      if (((x_301 < 10) & (x_303 <= x_304))) {
      } else {
        break;
      }
      int x_309 = k;
      k = (x_309 + 1);
      int x_311 = i;
      i = (x_311 + 1);
      int x_314 = data[x_311];
      temp[x_309] = x_314;
      {
      }
      continue;
    }
  }
  int x_316 = f;
  i_1 = x_316;
  {
    while(true) {
      int x_321 = i_1;
      int x_322 = to;
      if ((x_321 <= x_322)) {
      } else {
        break;
      }
      int x_325 = i_1;
      int x_326 = i_1;
      int x_328 = temp[x_326];
      data[x_325] = x_328;
      {
        int x_330 = i_1;
        i_1 = (x_330 + 1);
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
      int x_337 = m;
      int x_338 = high;
      if ((x_337 <= x_338)) {
      } else {
        break;
      }
      int x_341 = low;
      i_2 = x_341;
      {
        while(true) {
          int x_346 = i_2;
          int x_347 = high;
          if ((x_346 < x_347)) {
          } else {
            break;
          }
          int x_350 = i_2;
          f_1 = x_350;
          int x_351 = i_2;
          int x_352 = m;
          mid_1 = ((x_351 + x_352) - 1);
          int x_355 = i_2;
          int x_356 = m;
          int x_360 = high;
          to_1 = min(((x_355 + (2 * x_356)) - 1), x_360);
          int x_362 = f_1;
          param = x_362;
          int x_363 = mid_1;
          param_1 = x_363;
          int x_364 = to_1;
          param_2 = x_364;
          merge_i1_i1_i1_(param, param_1, param_2);
          {
            int x_366 = m;
            int x_368 = i_2;
            i_2 = (x_368 + (2 * x_366));
          }
          continue;
        }
      }
      {
        int x_370 = m;
        m = (2 * x_370);
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
  float x_89 = v.tint_symbol_3.injectionSwitch.x;
  i_3 = tint_f32_to_i32(x_89);
  {
    while(true) {
      int x_95 = i_3;
      switch(x_95) {
        case 9:
        {
          int x_125 = i_3;
          data[x_125] = -5;
          break;
        }
        case 8:
        {
          int x_123 = i_3;
          data[x_123] = -4;
          break;
        }
        case 7:
        {
          int x_121 = i_3;
          data[x_121] = -3;
          break;
        }
        case 6:
        {
          int x_119 = i_3;
          data[x_119] = -2;
          break;
        }
        case 5:
        {
          int x_117 = i_3;
          data[x_117] = -1;
          break;
        }
        case 4:
        {
          int x_115 = i_3;
          data[x_115] = 0;
          break;
        }
        case 3:
        {
          int x_113 = i_3;
          data[x_113] = 1;
          break;
        }
        case 2:
        {
          int x_111 = i_3;
          data[x_111] = 2;
          break;
        }
        case 1:
        {
          int x_109 = i_3;
          data[x_109] = 3;
          break;
        }
        case 0:
        {
          int x_107 = i_3;
          data[x_107] = 4;
          break;
        }
        default:
        {
          break;
        }
      }
      int x_127 = i_3;
      i_3 = (x_127 + 1);
      {
        int x_129 = i_3;
        if (!((x_129 < 10))) { break; }
      }
      continue;
    }
  }
  j_1 = 0;
  {
    while(true) {
      int x_135 = j_1;
      if ((x_135 < 10)) {
      } else {
        break;
      }
      int x_138 = j_1;
      int x_139 = j_1;
      int x_141 = data[x_139];
      temp[x_138] = x_141;
      {
        int x_143 = j_1;
        j_1 = (x_143 + 1);
      }
      continue;
    }
  }
  mergeSort_();
  float x_147 = tint_symbol.y;
  if ((tint_f32_to_i32(x_147) < 30)) {
    int x_154 = data[0];
    grey = (0.5f + (float(x_154) / 10.0f));
  } else {
    float x_159 = tint_symbol.y;
    if ((tint_f32_to_i32(x_159) < 60)) {
      int x_166 = data[1];
      grey = (0.5f + (float(x_166) / 10.0f));
    } else {
      float x_171 = tint_symbol.y;
      if ((tint_f32_to_i32(x_171) < 90)) {
        int x_178 = data[2];
        grey = (0.5f + (float(x_178) / 10.0f));
      } else {
        float x_183 = tint_symbol.y;
        if ((tint_f32_to_i32(x_183) < 120)) {
          int x_190 = data[3];
          grey = (0.5f + (float(x_190) / 10.0f));
        } else {
          float x_195 = tint_symbol.y;
          if ((tint_f32_to_i32(x_195) < 150)) {
            continue_execution = false;
          } else {
            float x_202 = tint_symbol.y;
            if ((tint_f32_to_i32(x_202) < 180)) {
              int x_209 = data[5];
              grey = (0.5f + (float(x_209) / 10.0f));
            } else {
              float x_214 = tint_symbol.y;
              if ((tint_f32_to_i32(x_214) < 210)) {
                int x_221 = data[6];
                grey = (0.5f + (float(x_221) / 10.0f));
              } else {
                float x_226 = tint_symbol.y;
                if ((tint_f32_to_i32(x_226) < 240)) {
                  int x_233 = data[7];
                  grey = (0.5f + (float(x_233) / 10.0f));
                } else {
                  float x_238 = tint_symbol.y;
                  if ((tint_f32_to_i32(x_238) < 270)) {
                    int x_245 = data[8];
                    grey = (0.5f + (float(x_245) / 10.0f));
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
  float x_249 = grey;
  vec3 x_250 = vec3(x_249, x_249, x_249);
  x_GLF_color = vec4(x_250[0u], x_250[1u], x_250[2u], 1.0f);
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
