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
  int x_262 = f;
  k = x_262;
  int x_263 = f;
  i = x_263;
  int x_264 = mid;
  j = (x_264 + 1);
  {
    while(true) {
      int x_270 = i;
      int x_271 = mid;
      int x_273 = j;
      int x_274 = to;
      if (((x_270 <= x_271) & (x_273 <= x_274))) {
      } else {
        break;
      }
      int x_278 = i;
      int x_280 = data[x_278];
      int x_281 = j;
      int x_283 = data[x_281];
      if ((x_280 < x_283)) {
        int x_288 = k;
        k = (x_288 + 1);
        int x_290 = i;
        i = (x_290 + 1);
        int x_293 = data[x_290];
        temp[x_288] = x_293;
      } else {
        int x_295 = k;
        k = (x_295 + 1);
        int x_297 = j;
        j = (x_297 + 1);
        int x_300 = data[x_297];
        temp[x_295] = x_300;
      }
      {
      }
      continue;
    }
  }
  {
    while(true) {
      int x_306 = i;
      int x_308 = i;
      int x_309 = mid;
      if (((x_306 < 10) & (x_308 <= x_309))) {
      } else {
        break;
      }
      int x_313 = k;
      k = (x_313 + 1);
      int x_315 = i;
      i = (x_315 + 1);
      int x_318 = data[x_315];
      temp[x_313] = x_318;
      {
      }
      continue;
    }
  }
  int x_320 = f;
  i_1 = x_320;
  {
    while(true) {
      int x_325 = i_1;
      int x_326 = to;
      if ((x_325 <= x_326)) {
      } else {
        break;
      }
      int x_329 = i_1;
      int x_330 = i_1;
      int x_332 = temp[x_330];
      data[x_329] = x_332;
      {
        int x_334 = i_1;
        i_1 = (x_334 + 1);
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
      int x_341 = m;
      int x_342 = high;
      if ((x_341 <= x_342)) {
      } else {
        break;
      }
      int x_345 = low;
      i_2 = x_345;
      {
        while(true) {
          int x_350 = i_2;
          int x_351 = high;
          if ((x_350 < x_351)) {
          } else {
            break;
          }
          int x_354 = i_2;
          f_1 = x_354;
          int x_355 = i_2;
          int x_356 = m;
          mid_1 = ((x_355 + x_356) - 1);
          int x_359 = i_2;
          int x_360 = m;
          int x_364 = high;
          to_1 = min(((x_359 + (2 * x_360)) - 1), x_364);
          int x_366 = f_1;
          param = x_366;
          int x_367 = mid_1;
          param_1 = x_367;
          int x_368 = to_1;
          param_2 = x_368;
          merge_i1_i1_i1_(param, param_1, param_2);
          {
            int x_370 = m;
            int x_372 = i_2;
            i_2 = (x_372 + (2 * x_370));
          }
          continue;
        }
      }
      {
        int x_374 = m;
        m = (2 * x_374);
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
  float x_86 = v.tint_symbol_3.injectionSwitch.x;
  i_3 = tint_f32_to_i32(x_86);
  {
    while(true) {
      int x_92 = i_3;
      switch(x_92) {
        case 9:
        {
          int x_122 = i_3;
          data[x_122] = -5;
          break;
        }
        case 8:
        {
          int x_120 = i_3;
          data[x_120] = -4;
          break;
        }
        case 7:
        {
          int x_118 = i_3;
          data[x_118] = -3;
          break;
        }
        case 6:
        {
          int x_116 = i_3;
          data[x_116] = -2;
          break;
        }
        case 5:
        {
          int x_114 = i_3;
          data[x_114] = -1;
          break;
        }
        case 4:
        {
          int x_112 = i_3;
          data[x_112] = 0;
          break;
        }
        case 3:
        {
          int x_110 = i_3;
          data[x_110] = 1;
          break;
        }
        case 2:
        {
          int x_108 = i_3;
          data[x_108] = 2;
          break;
        }
        case 1:
        {
          int x_106 = i_3;
          data[x_106] = 3;
          break;
        }
        case 0:
        {
          int x_104 = i_3;
          data[x_104] = 4;
          break;
        }
        default:
        {
          break;
        }
      }
      int x_124 = i_3;
      i_3 = (x_124 + 1);
      {
        int x_126 = i_3;
        if (!((x_126 < 10))) { break; }
      }
      continue;
    }
  }
  j_1 = 0;
  {
    while(true) {
      int x_132 = j_1;
      bool x_133 = (x_132 < 10);
      float x_135 = v.tint_symbol_3.injectionSwitch.x;
      if (!((x_135 <= 1.0f))) {
        grey = 1.0f;
      }
      if (x_133) {
      } else {
        break;
      }
      int x_140 = j_1;
      int x_141 = j_1;
      int x_143 = data[x_141];
      temp[x_140] = x_143;
      {
        int x_145 = j_1;
        j_1 = (x_145 + 1);
      }
      continue;
    }
  }
  mergeSort_();
  float x_149 = tint_symbol.y;
  if ((tint_f32_to_i32(x_149) < 30)) {
    int x_156 = data[0];
    grey = (0.5f + (float(x_156) / 10.0f));
  } else {
    float x_161 = tint_symbol.y;
    if ((tint_f32_to_i32(x_161) < 60)) {
      int x_168 = data[1];
      grey = (0.5f + (float(x_168) / 10.0f));
    } else {
      float x_173 = tint_symbol.y;
      if ((tint_f32_to_i32(x_173) < 90)) {
        int x_180 = data[2];
        grey = (0.5f + (float(x_180) / 10.0f));
      } else {
        float x_185 = tint_symbol.y;
        if ((tint_f32_to_i32(x_185) < 120)) {
          int x_192 = data[3];
          grey = (0.5f + (float(x_192) / 10.0f));
        } else {
          float x_197 = tint_symbol.y;
          if ((tint_f32_to_i32(x_197) < 150)) {
            continue_execution = false;
          } else {
            float x_204 = tint_symbol.y;
            if ((tint_f32_to_i32(x_204) < 180)) {
              int x_211 = data[5];
              grey = (0.5f + (float(x_211) / 10.0f));
            } else {
              float x_216 = tint_symbol.y;
              if ((tint_f32_to_i32(x_216) < 210)) {
                int x_223 = data[6];
                grey = (0.5f + (float(x_223) / 10.0f));
              } else {
                float x_228 = tint_symbol.y;
                if ((tint_f32_to_i32(x_228) < 240)) {
                  int x_235 = data[7];
                  grey = (0.5f + (float(x_235) / 10.0f));
                } else {
                  float x_240 = tint_symbol.y;
                  bool guard233 = true;
                  if ((tint_f32_to_i32(x_240) < 270)) {
                    int x_247 = data[8];
                    grey = (0.5f + (float(x_247) / 10.0f));
                    guard233 = false;
                  } else {
                    if (guard233) {
                      float x_252 = v.tint_symbol_3.injectionSwitch.y;
                      if (!((0.0f < x_252))) {
                        guard233 = false;
                      }
                      if (guard233) {
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
    }
  }
  float x_255 = grey;
  vec3 x_256 = vec3(x_255, x_255, x_255);
  x_GLF_color = vec4(x_256[0u], x_256[1u], x_256[2u], 1.0f);
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
