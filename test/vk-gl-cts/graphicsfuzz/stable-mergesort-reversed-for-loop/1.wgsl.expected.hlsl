static int data[10] = (int[10])0;
static int temp[10] = (int[10])0;
cbuffer cbuffer_x_28 : register(b0, space0) {
  uint4 x_28[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void merge_i1_i1_i1_(inout int from, inout int mid, inout int to) {
  int k = 0;
  int i = 0;
  int j = 0;
  int i_1 = 0;
  const int x_262 = from;
  k = x_262;
  const int x_263 = from;
  i = x_263;
  const int x_264 = mid;
  j = (x_264 + 1);
  while (true) {
    const int x_270 = i;
    const int x_271 = mid;
    const int x_273 = j;
    const int x_274 = to;
    bool tint_tmp = (x_270 <= x_271);
    if (tint_tmp) {
      tint_tmp = (x_273 <= x_274);
    }
    if ((tint_tmp)) {
    } else {
      break;
    }
    const int x_280 = data[i];
    const int x_283 = data[j];
    if ((x_280 < x_283)) {
      const int x_288 = k;
      k = (x_288 + 1);
      const int x_290 = i;
      i = (x_290 + 1);
      const int x_293 = data[x_290];
      temp[x_288] = x_293;
    } else {
      const int x_295 = k;
      k = (x_295 + 1);
      const int x_297 = j;
      j = (x_297 + 1);
      const int x_300 = data[x_297];
      temp[x_295] = x_300;
    }
  }
  while (true) {
    const int x_306 = i;
    const int x_308 = i;
    const int x_309 = mid;
    bool tint_tmp_1 = (x_306 < 10);
    if (tint_tmp_1) {
      tint_tmp_1 = (x_308 <= x_309);
    }
    if ((tint_tmp_1)) {
    } else {
      break;
    }
    const int x_313 = k;
    k = (x_313 + 1);
    const int x_315 = i;
    i = (x_315 + 1);
    const int x_318 = data[x_315];
    temp[x_313] = x_318;
  }
  const int x_320 = from;
  i_1 = x_320;
  while (true) {
    const int x_325 = i_1;
    const int x_326 = to;
    if ((x_325 <= x_326)) {
    } else {
      break;
    }
    const int x_329 = i_1;
    const int x_332 = temp[i_1];
    data[x_329] = x_332;
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
  int int_i = 0;
  const float x_85 = asfloat(x_28[0].x);
  i_3 = int(x_85);
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
      const int x_134 = j_1;
      const int x_137 = data[j_1];
      temp[x_134] = x_137;
    }
  }
  mergeSort_();
  const float x_143 = gl_FragCoord.y;
  if ((int(x_143) < 30)) {
    const int x_150 = data[0];
    grey = (0.5f + (float(x_150) / 10.0f));
  } else {
    const float x_155 = gl_FragCoord.y;
    if ((int(x_155) < 60)) {
      const int x_162 = data[1];
      grey = (0.5f + (float(x_162) / 10.0f));
    } else {
      const float x_167 = gl_FragCoord.y;
      if ((int(x_167) < 90)) {
        const int x_174 = data[2];
        grey = (0.5f + (float(x_174) / 10.0f));
      } else {
        const float x_179 = gl_FragCoord.y;
        if ((int(x_179) < 120)) {
          const int x_186 = data[3];
          grey = (0.5f + (float(x_186) / 10.0f));
        } else {
          const float x_191 = gl_FragCoord.y;
          if ((int(x_191) < 150)) {
            int_i = 1;
            while (true) {
              const int x_201 = int_i;
              const float x_203 = asfloat(x_28[0].x);
              if ((x_201 > int(x_203))) {
              } else {
                break;
              }
              discard;
            }
          } else {
            const float x_208 = gl_FragCoord.y;
            if ((int(x_208) < 180)) {
              const int x_215 = data[5];
              grey = (0.5f + (float(x_215) / 10.0f));
            } else {
              const float x_220 = gl_FragCoord.y;
              if ((int(x_220) < 210)) {
                const int x_227 = data[6];
                grey = (0.5f + (float(x_227) / 10.0f));
              } else {
                const float x_232 = gl_FragCoord.y;
                if ((int(x_232) < 240)) {
                  const int x_239 = data[7];
                  grey = (0.5f + (float(x_239) / 10.0f));
                } else {
                  const float x_244 = gl_FragCoord.y;
                  if ((int(x_244) < 270)) {
                    const int x_251 = data[8];
                    grey = (0.5f + (float(x_251) / 10.0f));
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
  const float x_255 = grey;
  const float3 x_256 = float3(x_255, x_255, x_255);
  x_GLF_color = float4(x_256.x, x_256.y, x_256.z, 1.0f);
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
