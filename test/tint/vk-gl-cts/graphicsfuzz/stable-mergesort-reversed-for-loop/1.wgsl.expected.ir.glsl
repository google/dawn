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
  int int_i = 0;
  float x_85 = v.tint_symbol_3.injectionSwitch.x;
  i_3 = tint_f32_to_i32(x_85);
  {
    while(true) {
      int x_91 = i_3;
      switch(x_91) {
        case 9:
        {
          int x_121 = i_3;
          data[x_121] = -5;
          break;
        }
        case 8:
        {
          int x_119 = i_3;
          data[x_119] = -4;
          break;
        }
        case 7:
        {
          int x_117 = i_3;
          data[x_117] = -3;
          break;
        }
        case 6:
        {
          int x_115 = i_3;
          data[x_115] = -2;
          break;
        }
        case 5:
        {
          int x_113 = i_3;
          data[x_113] = -1;
          break;
        }
        case 4:
        {
          int x_111 = i_3;
          data[x_111] = 0;
          break;
        }
        case 3:
        {
          int x_109 = i_3;
          data[x_109] = 1;
          break;
        }
        case 2:
        {
          int x_107 = i_3;
          data[x_107] = 2;
          break;
        }
        case 1:
        {
          int x_105 = i_3;
          data[x_105] = 3;
          break;
        }
        case 0:
        {
          int x_103 = i_3;
          data[x_103] = 4;
          break;
        }
        default:
        {
          break;
        }
      }
      int x_123 = i_3;
      i_3 = (x_123 + 1);
      {
        int x_125 = i_3;
        if (!((x_125 < 10))) { break; }
      }
      continue;
    }
  }
  j_1 = 0;
  {
    while(true) {
      int x_131 = j_1;
      if ((x_131 < 10)) {
      } else {
        break;
      }
      int x_134 = j_1;
      int x_135 = j_1;
      int x_137 = data[x_135];
      temp[x_134] = x_137;
      {
        int x_139 = j_1;
        j_1 = (x_139 + 1);
      }
      continue;
    }
  }
  mergeSort_();
  float x_143 = tint_symbol.y;
  if ((tint_f32_to_i32(x_143) < 30)) {
    int x_150 = data[0];
    grey = (0.5f + (float(x_150) / 10.0f));
  } else {
    float x_155 = tint_symbol.y;
    if ((tint_f32_to_i32(x_155) < 60)) {
      int x_162 = data[1];
      grey = (0.5f + (float(x_162) / 10.0f));
    } else {
      float x_167 = tint_symbol.y;
      if ((tint_f32_to_i32(x_167) < 90)) {
        int x_174 = data[2];
        grey = (0.5f + (float(x_174) / 10.0f));
      } else {
        float x_179 = tint_symbol.y;
        if ((tint_f32_to_i32(x_179) < 120)) {
          int x_186 = data[3];
          grey = (0.5f + (float(x_186) / 10.0f));
        } else {
          float x_191 = tint_symbol.y;
          if ((tint_f32_to_i32(x_191) < 150)) {
            int_i = 1;
            {
              while(true) {
                int x_201 = int_i;
                float x_203 = v.tint_symbol_3.injectionSwitch.x;
                if ((x_201 > tint_f32_to_i32(x_203))) {
                } else {
                  break;
                }
                continue_execution = false;
                {
                }
                continue;
              }
            }
          } else {
            float x_208 = tint_symbol.y;
            if ((tint_f32_to_i32(x_208) < 180)) {
              int x_215 = data[5];
              grey = (0.5f + (float(x_215) / 10.0f));
            } else {
              float x_220 = tint_symbol.y;
              if ((tint_f32_to_i32(x_220) < 210)) {
                int x_227 = data[6];
                grey = (0.5f + (float(x_227) / 10.0f));
              } else {
                float x_232 = tint_symbol.y;
                if ((tint_f32_to_i32(x_232) < 240)) {
                  int x_239 = data[7];
                  grey = (0.5f + (float(x_239) / 10.0f));
                } else {
                  float x_244 = tint_symbol.y;
                  if ((tint_f32_to_i32(x_244) < 270)) {
                    int x_251 = data[8];
                    grey = (0.5f + (float(x_251) / 10.0f));
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
