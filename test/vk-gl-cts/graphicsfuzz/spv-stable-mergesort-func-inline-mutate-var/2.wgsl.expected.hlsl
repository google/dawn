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
  const int x_302 = from;
  k = x_302;
  const int x_303 = from;
  i = x_303;
  const int x_304 = mid;
  j = (x_304 + 1);
  while (true) {
    const int x_310 = i;
    const int x_311 = mid;
    const int x_313 = j;
    const int x_314 = to;
    bool tint_tmp = (x_310 <= x_311);
    if (tint_tmp) {
      tint_tmp = (x_313 <= x_314);
    }
    if ((tint_tmp)) {
    } else {
      break;
    }
    const int x_320 = data[i];
    const int x_323 = data[j];
    if ((x_320 < x_323)) {
      const int x_328 = k;
      k = (x_328 + 1);
      const int x_330 = i;
      i = (x_330 + 1);
      const int x_333 = data[x_330];
      temp[x_328] = x_333;
    } else {
      const int x_335 = k;
      k = (x_335 + 1);
      const int x_337 = j;
      j = (x_337 + 1);
      const int x_340 = data[x_337];
      temp[x_335] = x_340;
    }
  }
  while (true) {
    const int x_346 = i;
    const int x_348 = i;
    const int x_349 = mid;
    bool tint_tmp_1 = (x_346 < 10);
    if (tint_tmp_1) {
      tint_tmp_1 = (x_348 <= x_349);
    }
    if ((tint_tmp_1)) {
    } else {
      break;
    }
    const int x_353 = k;
    k = (x_353 + 1);
    const int x_355 = i;
    i = (x_355 + 1);
    const int x_358 = data[x_355];
    temp[x_353] = x_358;
  }
  const int x_360 = from;
  i_1 = x_360;
  while (true) {
    const int x_365 = i_1;
    const int x_366 = to;
    if ((x_365 <= x_366)) {
    } else {
      break;
    }
    const int x_369 = i_1;
    const int x_372 = temp[i_1];
    data[x_369] = x_372;
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
          x_89 = ((x_91 + x_92) - 1);
          x_88 = min(((x_91 + (2 * x_92)) - 1), x_93);
          x_87 = x_90;
          x_86 = x_89;
          x_85 = x_88;
          merge_i1_i1_i1_(x_87, x_86, x_85);
        }
      }
    }
  }
  const float x_193 = gl_FragCoord.y;
  if ((int(x_193) < 30)) {
    const int x_200 = data[0];
    grey = (0.5f + (float(x_200) / 10.0f));
  } else {
    const float x_205 = gl_FragCoord.y;
    if ((int(x_205) < 60)) {
      const int x_212 = data[1];
      grey = (0.5f + (float(x_212) / 10.0f));
    } else {
      const float x_217 = gl_FragCoord.y;
      if ((int(x_217) < 90)) {
        const int x_224 = data[2];
        grey = (0.5f + (float(x_224) / 10.0f));
      } else {
        const float x_229 = gl_FragCoord.y;
        if ((int(x_229) < 120)) {
          const int x_236 = data[3];
          grey = (0.5f + (float(x_236) / 10.0f));
        } else {
          const float x_241 = gl_FragCoord.y;
          if ((int(x_241) < 150)) {
            discard;
          } else {
            const float x_248 = gl_FragCoord.y;
            if ((int(x_248) < 180)) {
              const int x_255 = data[5];
              grey = (0.5f + (float(x_255) / 10.0f));
            } else {
              const float x_260 = gl_FragCoord.y;
              if ((int(x_260) < 210)) {
                const int x_267 = data[6];
                grey = (0.5f + (float(x_267) / 10.0f));
              } else {
                const float x_272 = gl_FragCoord.y;
                if ((int(x_272) < 240)) {
                  const int x_279 = data[7];
                  grey = (0.5f + (float(x_279) / 10.0f));
                } else {
                  const float x_284 = gl_FragCoord.y;
                  if ((int(x_284) < 270)) {
                    const int x_291 = data[8];
                    grey = (0.5f + (float(x_291) / 10.0f));
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
  const float x_295 = grey;
  const float3 x_296 = float3(x_295, x_295, x_295);
  x_GLF_color = float4(x_296.x, x_296.y, x_296.z, 1.0f);
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
