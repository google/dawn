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
  const int x_303 = from;
  k = x_303;
  const int x_304 = from;
  i = x_304;
  const int x_305 = mid;
  j = (x_305 + 1);
  while (true) {
    const int x_311 = i;
    const int x_312 = mid;
    const int x_314 = j;
    const int x_315 = to;
    bool tint_tmp = (x_311 <= x_312);
    if (tint_tmp) {
      tint_tmp = (x_314 <= x_315);
    }
    if ((tint_tmp)) {
    } else {
      break;
    }
    const int x_321 = data[i];
    const int x_324 = data[j];
    if ((x_321 < x_324)) {
      const int x_329 = k;
      k = (x_329 + 1);
      const int x_331 = i;
      i = (x_331 + 1);
      const int x_334 = data[x_331];
      temp[x_329] = x_334;
    } else {
      const int x_336 = k;
      k = (x_336 + 1);
      const int x_338 = j;
      j = (x_338 + 1);
      const int x_341 = data[x_338];
      temp[x_336] = x_341;
    }
  }
  while (true) {
    const int x_347 = i;
    const int x_349 = i;
    const int x_350 = mid;
    bool tint_tmp_1 = (x_347 < 10);
    if (tint_tmp_1) {
      tint_tmp_1 = (x_349 <= x_350);
    }
    if ((tint_tmp_1)) {
    } else {
      break;
    }
    const int x_354 = k;
    k = (x_354 + 1);
    const int x_356 = i;
    i = (x_356 + 1);
    const int x_359 = data[x_356];
    temp[x_354] = x_359;
  }
  const int x_361 = from;
  i_1 = x_361;
  while (true) {
    const int x_366 = i_1;
    const int x_367 = to;
    if ((x_366 <= x_367)) {
    } else {
      break;
    }
    const int x_370 = i_1;
    const int x_373 = temp[i_1];
    data[x_370] = x_373;
    {
      i_1 = (i_1 + 1);
    }
  }
  return;
}

void main_1() {
  int x_85 = 0;
  int x_86 = 0;
  int x_87 = 0;
  int x_88 = 0;
  int x_89 = 0;
  int x_90 = 0;
  int x_91 = 0;
  int x_92 = 0;
  int x_93 = 0;
  int x_94 = 0;
  int i_3 = 0;
  int j_1 = 0;
  float grey = 0.0f;
  const float x_96 = asfloat(x_28[0].x);
  i_3 = int(x_96);
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
      const int x_145 = j_1;
      const int x_148 = data[j_1];
      temp[x_145] = x_148;
    }
  }
  x_94 = 0;
  x_93 = 9;
  x_92 = 1;
  {
    for(; (x_92 <= x_93); x_92 = (2 * x_92)) {
      x_91 = x_94;
      {
        for(; (x_91 < x_93); x_91 = (x_91 + (2 * x_92))) {
          x_90 = x_91;
          const int x_170 = x_91;
          const int x_171 = x_92;
          const int x_173[10] = data;
          const int tint_symbol_4[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
          data = tint_symbol_4;
          data = x_173;
          x_89 = ((x_170 + x_171) - 1);
          x_88 = min(((x_91 + (2 * x_92)) - 1), x_93);
          x_87 = x_90;
          x_86 = x_89;
          x_85 = x_88;
          merge_i1_i1_i1_(x_87, x_86, x_85);
        }
      }
    }
  }
  const float x_194 = gl_FragCoord.y;
  if ((int(x_194) < 30)) {
    const int x_201 = data[0];
    grey = (0.5f + (float(x_201) / 10.0f));
  } else {
    const float x_206 = gl_FragCoord.y;
    if ((int(x_206) < 60)) {
      const int x_213 = data[1];
      grey = (0.5f + (float(x_213) / 10.0f));
    } else {
      const float x_218 = gl_FragCoord.y;
      if ((int(x_218) < 90)) {
        const int x_225 = data[2];
        grey = (0.5f + (float(x_225) / 10.0f));
      } else {
        const float x_230 = gl_FragCoord.y;
        if ((int(x_230) < 120)) {
          const int x_237 = data[3];
          grey = (0.5f + (float(x_237) / 10.0f));
        } else {
          const float x_242 = gl_FragCoord.y;
          if ((int(x_242) < 150)) {
            discard;
          } else {
            const float x_249 = gl_FragCoord.y;
            if ((int(x_249) < 180)) {
              const int x_256 = data[5];
              grey = (0.5f + (float(x_256) / 10.0f));
            } else {
              const float x_261 = gl_FragCoord.y;
              if ((int(x_261) < 210)) {
                const int x_268 = data[6];
                grey = (0.5f + (float(x_268) / 10.0f));
              } else {
                const float x_273 = gl_FragCoord.y;
                if ((int(x_273) < 240)) {
                  const int x_280 = data[7];
                  grey = (0.5f + (float(x_280) / 10.0f));
                } else {
                  const float x_285 = gl_FragCoord.y;
                  if ((int(x_285) < 270)) {
                    const int x_292 = data[8];
                    grey = (0.5f + (float(x_292) / 10.0f));
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
  const float x_296 = grey;
  const float3 x_297 = float3(x_296, x_296, x_296);
  x_GLF_color = float4(x_297.x, x_297.y, x_297.z, 1.0f);
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
  const main_out tint_symbol_5 = {x_GLF_color};
  return tint_symbol_5;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
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
