static int data[10] = (int[10])0;
static int temp[10] = (int[10])0;
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_34 : register(b0, space0) {
  uint4 x_34[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void merge_i1_i1_i1_(inout int from, inout int mid, inout int to) {
  int k = 0;
  int i = 0;
  int j = 0;
  int i_1 = 0;
  const int x_260 = from;
  k = x_260;
  const int x_261 = from;
  i = x_261;
  const int x_262 = mid;
  j = (x_262 + 1);
  while (true) {
    const int x_268 = i;
    const int x_269 = mid;
    const int x_271 = j;
    const int x_272 = to;
    if (((x_268 <= x_269) & (x_271 <= x_272))) {
    } else {
      break;
    }
    const int x_278 = data[i];
    const int x_281 = data[j];
    if ((x_278 < x_281)) {
      const int x_286 = k;
      k = (x_286 + 1);
      const int x_288 = i;
      i = (x_288 + 1);
      const int x_291 = data[x_288];
      temp[x_286] = x_291;
    } else {
      const int x_293 = k;
      k = (x_293 + 1);
      const int x_295 = j;
      j = (x_295 + 1);
      const int x_298 = data[x_295];
      temp[x_293] = x_298;
    }
  }
  while (true) {
    const int x_304 = i;
    const int x_306 = i;
    const int x_307 = mid;
    if (((x_304 < 10) & (x_306 <= x_307))) {
    } else {
      break;
    }
    const int x_311 = k;
    k = (x_311 + 1);
    const int x_313 = i;
    i = (x_313 + 1);
    const int x_316 = data[x_313];
    temp[x_311] = x_316;
  }
  const int x_318 = from;
  i_1 = x_318;
  while (true) {
    const int x_323 = i_1;
    const int x_324 = to;
    if ((x_323 <= x_324)) {
    } else {
      break;
    }
    const int x_327 = i_1;
    const int x_330 = temp[i_1];
    data[x_327] = x_330;
    {
      i_1 = (i_1 + 1);
    }
  }
  return;
}

int func_i1_i1_(inout int m, inout int high) {
  int x = 0;
  int x_335 = 0;
  int x_336 = 0;
  const float x_338 = gl_FragCoord.x;
  if ((x_338 >= 0.0f)) {
    if (false) {
      const int x_346 = high;
      x_336 = (x_346 << asuint(0));
    } else {
      x_336 = 4;
    }
    x_335 = (1 << asuint(x_336));
  } else {
    x_335 = 1;
  }
  x = x_335;
  x = (x >> asuint(4));
  const int x_353 = m;
  const int x_355 = m;
  const int x_357 = m;
  return clamp((2 * x_353), (2 * x_355), ((2 * x_357) / x));
}

void mergeSort_() {
  int low = 0;
  int high_1 = 0;
  int m_1 = 0;
  int i_2 = 0;
  int from_1 = 0;
  int mid_1 = 0;
  int to_1 = 0;
  int param = 0;
  int param_1 = 0;
  int param_2 = 0;
  int param_3 = 0;
  int param_4 = 0;
  low = 0;
  high_1 = 9;
  m_1 = 1;
  {
    for(; (m_1 <= high_1); m_1 = (2 * m_1)) {
      i_2 = low;
      while (true) {
        if ((i_2 < high_1)) {
        } else {
          break;
        }
        from_1 = i_2;
        mid_1 = ((i_2 + m_1) - 1);
        to_1 = min(((i_2 + (2 * m_1)) - 1), high_1);
        param = from_1;
        param_1 = mid_1;
        param_2 = to_1;
        merge_i1_i1_i1_(param, param_1, param_2);
        {
          param_3 = m_1;
          param_4 = high_1;
          const int x_398 = func_i1_i1_(param_3, param_4);
          i_2 = (i_2 + x_398);
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
  const float x_93 = asfloat(x_34[0].x);
  i_3 = int(x_93);
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
      const int x_142 = j_1;
      const int x_145 = data[j_1];
      temp[x_142] = x_145;
    }
  }
  mergeSort_();
  const float x_151 = gl_FragCoord.y;
  if ((int(x_151) < 30)) {
    const int x_158 = data[0];
    grey = (0.5f + (float(x_158) / 10.0f));
  } else {
    const float x_163 = gl_FragCoord.y;
    if ((int(x_163) < 60)) {
      const int x_170 = data[1];
      grey = (0.5f + (float(x_170) / 10.0f));
    } else {
      const float x_175 = gl_FragCoord.y;
      if ((int(x_175) < 90)) {
        const int x_182 = data[2];
        grey = (0.5f + (float(x_182) / 10.0f));
      } else {
        const float x_187 = gl_FragCoord.y;
        if ((int(x_187) < 120)) {
          const int x_194 = data[3];
          grey = (0.5f + (float(x_194) / 10.0f));
        } else {
          const float x_199 = gl_FragCoord.y;
          if ((int(x_199) < 150)) {
            discard;
          } else {
            const float x_206 = gl_FragCoord.y;
            if ((int(x_206) < 180)) {
              const int x_213 = data[5];
              grey = (0.5f + (float(x_213) / 10.0f));
            } else {
              const float x_218 = gl_FragCoord.y;
              if ((int(x_218) < 210)) {
                const int x_225 = data[6];
                grey = (0.5f + (float(x_225) / 10.0f));
              } else {
                const float x_230 = gl_FragCoord.y;
                if ((int(x_230) < 240)) {
                  const int x_237 = data[7];
                  grey = (0.5f + (float(x_237) / 10.0f));
                } else {
                  const float x_242 = gl_FragCoord.y;
                  if ((int(x_242) < 270)) {
                    const int x_249 = data[8];
                    grey = (0.5f + (float(x_249) / 10.0f));
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
  const float x_253 = grey;
  const float3 x_254 = float3(x_253, x_253, x_253);
  x_GLF_color = float4(x_254.x, x_254.y, x_254.z, 1.0f);
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
