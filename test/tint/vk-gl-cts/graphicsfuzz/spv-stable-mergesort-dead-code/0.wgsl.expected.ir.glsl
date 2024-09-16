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
  int x_251 = f;
  k = x_251;
  int x_252 = f;
  i = x_252;
  int x_253 = mid;
  j = (x_253 + 1);
  {
    while(true) {
      int x_259 = i;
      int x_260 = mid;
      int x_262 = j;
      int x_263 = to;
      if (((x_259 <= x_260) & (x_262 <= x_263))) {
      } else {
        break;
      }
      int x_267 = i;
      int x_269 = data[x_267];
      int x_270 = j;
      int x_272 = data[x_270];
      if ((x_269 < x_272)) {
        int x_277 = k;
        k = (x_277 + 1);
        int x_279 = i;
        i = (x_279 + 1);
        int x_282 = data[x_279];
        temp[x_277] = x_282;
      } else {
        int x_284 = k;
        k = (x_284 + 1);
        int x_286 = j;
        j = (x_286 + 1);
        int x_289 = data[x_286];
        temp[x_284] = x_289;
      }
      {
      }
      continue;
    }
  }
  {
    while(true) {
      int x_295 = i;
      int x_297 = i;
      int x_298 = mid;
      if (((x_295 < 10) & (x_297 <= x_298))) {
      } else {
        break;
      }
      int x_302 = k;
      k = (x_302 + 1);
      int x_304 = i;
      i = (x_304 + 1);
      int x_307 = data[x_304];
      temp[x_302] = x_307;
      {
      }
      continue;
    }
  }
  int x_309 = f;
  i_1 = x_309;
  {
    while(true) {
      int x_314 = i_1;
      int x_315 = to;
      if ((x_314 <= x_315)) {
      } else {
        break;
      }
      int x_318 = i_1;
      int x_319 = i_1;
      int x_321 = temp[x_319];
      data[x_318] = x_321;
      {
        int x_323 = i_1;
        i_1 = (x_323 + 1);
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
      int x_330 = m;
      int x_331 = high;
      if ((x_330 <= x_331)) {
      } else {
        break;
      }
      int x_334 = low;
      i_2 = x_334;
      {
        while(true) {
          int x_339 = i_2;
          int x_340 = high;
          if ((x_339 < x_340)) {
          } else {
            break;
          }
          int x_343 = i_2;
          f_1 = x_343;
          int x_344 = i_2;
          int x_345 = m;
          mid_1 = ((x_344 + x_345) - 1);
          int x_348 = i_2;
          int x_349 = m;
          int x_353 = high;
          to_1 = min(((x_348 + (2 * x_349)) - 1), x_353);
          int x_355 = f_1;
          param = x_355;
          int x_356 = mid_1;
          param_1 = x_356;
          int x_357 = to_1;
          param_2 = x_357;
          merge_i1_i1_i1_(param, param_1, param_2);
          {
            int x_359 = m;
            int x_361 = i_2;
            i_2 = (x_361 + (2 * x_359));
          }
          continue;
        }
      }
      {
        int x_363 = m;
        m = (2 * x_363);
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
  float x_84 = v.tint_symbol_3.injectionSwitch.x;
  i_3 = tint_f32_to_i32(x_84);
  {
    while(true) {
      int x_90 = i_3;
      switch(x_90) {
        case 9:
        {
          int x_120 = i_3;
          data[x_120] = -5;
          break;
        }
        case 8:
        {
          int x_118 = i_3;
          data[x_118] = -4;
          break;
        }
        case 7:
        {
          int x_116 = i_3;
          data[x_116] = -3;
          break;
        }
        case 6:
        {
          int x_114 = i_3;
          data[x_114] = -2;
          break;
        }
        case 5:
        {
          int x_112 = i_3;
          data[x_112] = -1;
          break;
        }
        case 4:
        {
          int x_110 = i_3;
          data[x_110] = 0;
          break;
        }
        case 3:
        {
          int x_108 = i_3;
          data[x_108] = 1;
          break;
        }
        case 2:
        {
          int x_106 = i_3;
          data[x_106] = 2;
          break;
        }
        case 1:
        {
          int x_104 = i_3;
          data[x_104] = 3;
          break;
        }
        case 0:
        {
          int x_102 = i_3;
          data[x_102] = 4;
          break;
        }
        default:
        {
          break;
        }
      }
      int x_122 = i_3;
      i_3 = (x_122 + 1);
      {
        int x_124 = i_3;
        if (!((x_124 < 10))) { break; }
      }
      continue;
    }
  }
  j_1 = 0;
  {
    while(true) {
      int x_130 = j_1;
      if ((x_130 < 10)) {
      } else {
        break;
      }
      int x_133 = j_1;
      int x_134 = j_1;
      int x_136 = data[x_134];
      temp[x_133] = x_136;
      {
        int x_138 = j_1;
        j_1 = (x_138 + 1);
      }
      continue;
    }
  }
  mergeSort_();
  float x_142 = tint_symbol.y;
  if ((tint_f32_to_i32(x_142) < 30)) {
    int x_149 = data[0];
    grey = (0.5f + (float(x_149) / 10.0f));
  } else {
    float x_154 = tint_symbol.y;
    if ((tint_f32_to_i32(x_154) < 60)) {
      int x_161 = data[1];
      grey = (0.5f + (float(x_161) / 10.0f));
    } else {
      float x_166 = tint_symbol.y;
      if ((tint_f32_to_i32(x_166) < 90)) {
        int x_173 = data[2];
        grey = (0.5f + (float(x_173) / 10.0f));
      } else {
        float x_178 = tint_symbol.y;
        if ((tint_f32_to_i32(x_178) < 120)) {
          int x_185 = data[3];
          grey = (0.5f + (float(x_185) / 10.0f));
        } else {
          float x_190 = tint_symbol.y;
          if ((tint_f32_to_i32(x_190) < 150)) {
            continue_execution = false;
          } else {
            float x_197 = tint_symbol.y;
            if ((tint_f32_to_i32(x_197) < 180)) {
              int x_204 = data[5];
              grey = (0.5f + (float(x_204) / 10.0f));
            } else {
              float x_209 = tint_symbol.y;
              if ((tint_f32_to_i32(x_209) < 210)) {
                int x_216 = data[6];
                grey = (0.5f + (float(x_216) / 10.0f));
              } else {
                float x_221 = tint_symbol.y;
                if ((tint_f32_to_i32(x_221) < 240)) {
                  int x_228 = data[7];
                  grey = (0.5f + (float(x_228) / 10.0f));
                } else {
                  float x_233 = tint_symbol.y;
                  if ((tint_f32_to_i32(x_233) < 270)) {
                    int x_240 = data[8];
                    grey = (0.5f + (float(x_240) / 10.0f));
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
  float x_244 = grey;
  vec3 x_245 = vec3(x_244, x_244, x_244);
  x_GLF_color = vec4(x_245[0u], x_245[1u], x_245[2u], 1.0f);
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
