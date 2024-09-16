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
  int x_254 = f;
  k = x_254;
  int x_255 = f;
  i = x_255;
  int x_256 = mid;
  j = (x_256 + 1);
  {
    while(true) {
      int x_262 = i;
      int x_263 = mid;
      int x_265 = j;
      int x_266 = to;
      if (((x_262 <= x_263) & (x_265 <= x_266))) {
      } else {
        break;
      }
      int x_270 = i;
      int x_272 = data[x_270];
      int x_273 = j;
      int x_275 = data[x_273];
      if ((x_272 < x_275)) {
        int x_280 = k;
        k = (x_280 + 1);
        int x_282 = i;
        i = (x_282 + 1);
        int x_285 = data[x_282];
        temp[x_280] = x_285;
      } else {
        int x_287 = k;
        k = (x_287 + 1);
        int x_289 = j;
        j = (x_289 + 1);
        int x_292 = data[x_289];
        temp[x_287] = x_292;
      }
      {
      }
      continue;
    }
  }
  {
    while(true) {
      int x_298 = i;
      int x_300 = i;
      int x_301 = mid;
      if (((x_298 < 10) & (x_300 <= x_301))) {
      } else {
        break;
      }
      int x_305 = k;
      k = (x_305 + 1);
      int x_307 = i;
      i = (x_307 + 1);
      int x_310 = data[x_307];
      temp[x_305] = x_310;
      {
      }
      continue;
    }
  }
  int x_312 = f;
  i_1 = x_312;
  {
    while(true) {
      int x_317 = i_1;
      int x_318 = to;
      if ((x_317 <= x_318)) {
      } else {
        break;
      }
      int x_321 = i_1;
      int x_322 = i_1;
      int x_324 = temp[x_322];
      data[x_321] = x_324;
      {
        int x_326 = i_1;
        i_1 = (x_326 + 1);
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
      int x_333 = m;
      int x_334 = high;
      if ((x_333 <= x_334)) {
      } else {
        break;
      }
      int x_337 = low;
      i_2 = x_337;
      {
        while(true) {
          int x_342 = i_2;
          int x_343 = high;
          if ((x_342 < x_343)) {
          } else {
            break;
          }
          int x_346 = i_2;
          f_1 = x_346;
          int x_347 = i_2;
          int x_348 = m;
          mid_1 = ((x_347 + x_348) - 1);
          int x_351 = i_2;
          int x_352 = m;
          int x_356 = high;
          to_1 = min(((x_351 + (2 * x_352)) - 1), x_356);
          int x_358 = f_1;
          param = x_358;
          int x_359 = mid_1;
          param_1 = x_359;
          int x_360 = to_1;
          param_2 = x_360;
          merge_i1_i1_i1_(param, param_1, param_2);
          {
            int x_362 = m;
            int x_364 = i_2;
            i_2 = (x_364 + (2 * x_362));
          }
          continue;
        }
      }
      {
        int x_366 = m;
        m = (2 * x_366);
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
  float x_87 = v.tint_symbol_3.injectionSwitch.x;
  i_3 = tint_f32_to_i32(x_87);
  {
    while(true) {
      int x_93 = i_3;
      switch(x_93) {
        case 9:
        {
          int x_123 = i_3;
          data[x_123] = -5;
          break;
        }
        case 8:
        {
          int x_121 = i_3;
          data[x_121] = -4;
          break;
        }
        case 7:
        {
          int x_119 = i_3;
          data[x_119] = -3;
          break;
        }
        case 6:
        {
          int x_117 = i_3;
          data[x_117] = -2;
          break;
        }
        case 5:
        {
          int x_115 = i_3;
          data[x_115] = -1;
          break;
        }
        case 4:
        {
          int x_113 = i_3;
          data[x_113] = 0;
          break;
        }
        case 3:
        {
          int x_111 = i_3;
          data[x_111] = 1;
          break;
        }
        case 2:
        {
          int x_109 = i_3;
          data[x_109] = 2;
          break;
        }
        case 1:
        {
          int x_107 = i_3;
          data[x_107] = 3;
          break;
        }
        case 0:
        {
          int x_105 = i_3;
          data[x_105] = 4;
          break;
        }
        default:
        {
          break;
        }
      }
      int x_125 = i_3;
      i_3 = (x_125 + 1);
      {
        int x_127 = i_3;
        if (!((x_127 < 10))) { break; }
      }
      continue;
    }
  }
  j_1 = 0;
  {
    while(true) {
      int x_133 = j_1;
      if ((x_133 < 10)) {
      } else {
        break;
      }
      int x_136 = j_1;
      int x_137 = j_1;
      int x_139 = data[x_137];
      temp[x_136] = x_139;
      {
        int x_141 = j_1;
        j_1 = (x_141 + 1);
      }
      continue;
    }
  }
  mergeSort_();
  float x_145 = tint_symbol.y;
  if ((tint_f32_to_i32(x_145) < 30)) {
    int x_152 = data[0];
    grey = (0.5f + (float(x_152) / 10.0f));
  } else {
    float x_157 = tint_symbol.y;
    if ((tint_f32_to_i32(x_157) < 60)) {
      int x_164 = data[1];
      grey = (0.5f + (float(x_164) / 10.0f));
    } else {
      float x_169 = tint_symbol.y;
      if ((tint_f32_to_i32(x_169) < 90)) {
        int x_176 = data[2];
        grey = (0.5f + (float(x_176) / 10.0f));
      } else {
        float x_181 = tint_symbol.y;
        if ((tint_f32_to_i32(x_181) < 120)) {
          int x_188 = data[3];
          grey = (0.5f + (float(x_188) / 10.0f));
        } else {
          float x_193 = tint_symbol.y;
          if ((tint_f32_to_i32(x_193) < 150)) {
            continue_execution = false;
          } else {
            float x_200 = tint_symbol.y;
            if ((tint_f32_to_i32(x_200) < 180)) {
              int x_207 = data[5];
              grey = (0.5f + (float(x_207) / 10.0f));
            } else {
              float x_212 = tint_symbol.y;
              if ((tint_f32_to_i32(x_212) < 210)) {
                int x_219 = data[6];
                grey = (0.5f + (float(x_219) / 10.0f));
              } else {
                float x_224 = tint_symbol.y;
                if ((tint_f32_to_i32(x_224) < 240)) {
                  int x_231 = data[7];
                  grey = (0.5f + (float(x_231) / 10.0f));
                } else {
                  float x_236 = tint_symbol.y;
                  if ((tint_f32_to_i32(x_236) < 270)) {
                    int x_243 = data[8];
                    grey = (0.5f + (float(x_243) / 10.0f));
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
  float x_247 = grey;
  vec3 x_248 = vec3(x_247, x_247, x_247);
  x_GLF_color = vec4(x_248[0u], x_248[1u], x_248[2u], 1.0f);
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
