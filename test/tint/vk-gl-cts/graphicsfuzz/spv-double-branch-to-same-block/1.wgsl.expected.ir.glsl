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
      int x_263 = i;
      int x_264 = mid;
      int x_266 = j;
      int x_267 = to;
      if (((x_263 <= x_264) & (x_266 <= x_267))) {
      } else {
        break;
      }
      int x_271 = i;
      int x_273 = data[x_271];
      int x_274 = j;
      int x_276 = data[x_274];
      if ((x_273 < x_276)) {
        int x_281 = k;
        k = (x_281 + 1);
        int x_283 = i;
        i = (x_283 + 1);
        int x_286 = data[x_283];
        temp[x_281] = x_286;
      } else {
        int x_288 = k;
        k = (x_288 + 1);
        int x_290 = j;
        j = (x_290 + 1);
        int x_293 = data[x_290];
        temp[x_288] = x_293;
      }
      {
      }
      continue;
    }
  }
  {
    while(true) {
      int x_299 = i;
      int x_301 = i;
      int x_302 = mid;
      if (((x_299 < 10) & (x_301 <= x_302))) {
      } else {
        break;
      }
      int x_306 = k;
      k = (x_306 + 1);
      int x_308 = i;
      i = (x_308 + 1);
      int x_311 = data[x_308];
      temp[x_306] = x_311;
      {
      }
      continue;
    }
  }
  int x_313 = f;
  i_1 = x_313;
  {
    while(true) {
      int x_318 = i_1;
      int x_319 = to;
      if ((x_318 <= x_319)) {
      } else {
        break;
      }
      int x_322 = i_1;
      int x_323 = i_1;
      int x_325 = temp[x_323];
      data[x_322] = x_325;
      {
        int x_327 = i_1;
        i_1 = (x_327 + 1);
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
      int x_334 = m;
      int x_335 = high;
      if ((x_334 <= x_335)) {
      } else {
        break;
      }
      int x_338 = low;
      i_2 = x_338;
      {
        while(true) {
          int x_343 = i_2;
          int x_344 = high;
          if ((x_343 < x_344)) {
          } else {
            break;
          }
          int x_347 = i_2;
          f_1 = x_347;
          int x_348 = i_2;
          int x_349 = m;
          mid_1 = ((x_348 + x_349) - 1);
          int x_352 = i_2;
          int x_353 = m;
          int x_357 = high;
          to_1 = min(((x_352 + (2 * x_353)) - 1), x_357);
          int x_359 = f_1;
          param = x_359;
          int x_360 = mid_1;
          param_1 = x_360;
          int x_361 = to_1;
          param_2 = x_361;
          merge_i1_i1_i1_(param, param_1, param_2);
          {
            int x_363 = m;
            int x_365 = i_2;
            i_2 = (x_365 + (2 * x_363));
          }
          continue;
        }
      }
      {
        int x_367 = m;
        m = (2 * x_367);
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
      bool tint_continue = false;
      switch(x_94) {
        case 9:
        {
          int x_124 = i_3;
          data[x_124] = -5;
          if (true) {
          } else {
            tint_continue = true;
            break;
          }
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
      if (tint_continue) {
        {
          int x_128 = i_3;
          if (!((x_128 < 10))) { break; }
        }
        continue;
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
ERROR: 0:41: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:41: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
