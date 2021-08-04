static int data[10] = (int[10])0;
static int temp[10] = (int[10])0;
cbuffer cbuffer_x_28 : register(b0, space0) {
  uint4 x_28[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_32 : register(b1, space0) {
  uint4 x_32[1];
};

void merge_i1_i1_i1_(inout int from, inout int mid, inout int to) {
  int k = 0;
  int i = 0;
  int j = 0;
  int i_1 = 0;
  const int x_256 = from;
  k = x_256;
  const int x_257 = from;
  i = x_257;
  const int x_258 = mid;
  j = (x_258 + 1);
  while (true) {
    const int x_264 = i;
    const int x_265 = mid;
    const int x_267 = j;
    const int x_268 = to;
    bool tint_tmp = (x_264 <= x_265);
    if (tint_tmp) {
      tint_tmp = (x_267 <= x_268);
    }
    if ((tint_tmp)) {
    } else {
      break;
    }
    const int x_274 = data[i];
    const int x_277 = data[j];
    if ((x_274 < x_277)) {
      const int x_282 = k;
      k = (x_282 + 1);
      const int x_284 = i;
      i = (x_284 + 1);
      const int x_287 = data[x_284];
      temp[x_282] = x_287;
    } else {
      const int x_289 = k;
      k = (x_289 + 1);
      const int x_291 = j;
      j = (x_291 + 1);
      const int x_294 = data[x_291];
      temp[x_289] = x_294;
    }
  }
  while (true) {
    if (!((256.0f < 1.0f))) {
    } else {
      continue;
    }
    const int x_301 = i;
    const int x_303 = i;
    const int x_304 = mid;
    bool tint_tmp_1 = (x_301 < 10);
    if (tint_tmp_1) {
      tint_tmp_1 = (x_303 <= x_304);
    }
    if ((tint_tmp_1)) {
    } else {
      break;
    }
    const int x_309 = k;
    k = (x_309 + 1);
    const int x_311 = i;
    i = (x_311 + 1);
    const int x_314 = data[x_311];
    temp[x_309] = x_314;
  }
  const int x_316 = from;
  i_1 = x_316;
  while (true) {
    const int x_321 = i_1;
    const int x_322 = to;
    if ((x_321 <= x_322)) {
    } else {
      break;
    }
    const int x_325 = i_1;
    const int x_328 = temp[i_1];
    data[x_325] = x_328;
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
  const float x_89 = asfloat(x_28[0].x);
  i_3 = int(x_89);
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
      const int x_138 = j_1;
      const int x_141 = data[j_1];
      temp[x_138] = x_141;
    }
  }
  mergeSort_();
  const float x_147 = gl_FragCoord.y;
  if ((int(x_147) < 30)) {
    const int x_154 = data[0];
    grey = (0.5f + (float(x_154) / 10.0f));
  } else {
    const float x_159 = gl_FragCoord.y;
    if ((int(x_159) < 60)) {
      const int x_166 = data[1];
      grey = (0.5f + (float(x_166) / 10.0f));
    } else {
      const float x_171 = gl_FragCoord.y;
      if ((int(x_171) < 90)) {
        const int x_178 = data[2];
        grey = (0.5f + (float(x_178) / 10.0f));
      } else {
        const float x_183 = gl_FragCoord.y;
        if ((int(x_183) < 120)) {
          const int x_190 = data[3];
          grey = (0.5f + (float(x_190) / 10.0f));
        } else {
          const float x_195 = gl_FragCoord.y;
          if ((int(x_195) < 150)) {
            discard;
          } else {
            const float x_202 = gl_FragCoord.y;
            if ((int(x_202) < 180)) {
              const int x_209 = data[5];
              grey = (0.5f + (float(x_209) / 10.0f));
            } else {
              const float x_214 = gl_FragCoord.y;
              if ((int(x_214) < 210)) {
                const int x_221 = data[6];
                grey = (0.5f + (float(x_221) / 10.0f));
              } else {
                const float x_226 = gl_FragCoord.y;
                if ((int(x_226) < 240)) {
                  const int x_233 = data[7];
                  grey = (0.5f + (float(x_233) / 10.0f));
                } else {
                  const float x_238 = gl_FragCoord.y;
                  if ((int(x_238) < 270)) {
                    const int x_245 = data[8];
                    grey = (0.5f + (float(x_245) / 10.0f));
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
  const float x_249 = grey;
  const float3 x_250 = float3(x_249, x_249, x_249);
  x_GLF_color = float4(x_250.x, x_250.y, x_250.z, 1.0f);
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
