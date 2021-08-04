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
  const int x_251 = from;
  k = x_251;
  const int x_252 = from;
  i = x_252;
  const int x_253 = mid;
  j = (x_253 + 1);
  while (true) {
    const int x_259 = i;
    const int x_260 = mid;
    const int x_262 = j;
    const int x_263 = to;
    bool tint_tmp = (x_259 <= x_260);
    if (tint_tmp) {
      tint_tmp = (x_262 <= x_263);
    }
    if ((tint_tmp)) {
    } else {
      break;
    }
    const int x_269 = data[i];
    const int x_272 = data[j];
    if ((x_269 < x_272)) {
      const int x_277 = k;
      k = (x_277 + 1);
      const int x_279 = i;
      i = (x_279 + 1);
      const int x_282 = data[x_279];
      temp[x_277] = x_282;
    } else {
      const int x_284 = k;
      k = (x_284 + 1);
      const int x_286 = j;
      j = (x_286 + 1);
      const int x_289 = data[x_286];
      temp[x_284] = x_289;
    }
  }
  while (true) {
    const int x_295 = i;
    const int x_297 = i;
    const int x_298 = mid;
    bool tint_tmp_1 = (x_295 < 10);
    if (tint_tmp_1) {
      tint_tmp_1 = (x_297 <= x_298);
    }
    if ((tint_tmp_1)) {
    } else {
      break;
    }
    const int x_302 = k;
    k = (x_302 + 1);
    const int x_304 = i;
    i = (x_304 + 1);
    const int x_307 = data[x_304];
    temp[x_302] = x_307;
  }
  const int x_309 = from;
  i_1 = x_309;
  while (true) {
    const int x_314 = i_1;
    const int x_315 = to;
    if ((x_314 <= x_315)) {
    } else {
      break;
    }
    const int x_318 = i_1;
    const int x_321 = temp[i_1];
    data[x_318] = x_321;
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
  const float x_84 = asfloat(x_28[0].x);
  i_3 = int(x_84);
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
      const int x_133 = j_1;
      const int x_136 = data[j_1];
      temp[x_133] = x_136;
    }
  }
  mergeSort_();
  const float x_142 = gl_FragCoord.y;
  if ((int(x_142) < 30)) {
    const int x_149 = data[0];
    grey = (0.5f + (float(x_149) / 10.0f));
  } else {
    const float x_154 = gl_FragCoord.y;
    if ((int(x_154) < 60)) {
      const int x_161 = data[1];
      grey = (0.5f + (float(x_161) / 10.0f));
    } else {
      const float x_166 = gl_FragCoord.y;
      if ((int(x_166) < 90)) {
        const int x_173 = data[2];
        grey = (0.5f + (float(x_173) / 10.0f));
      } else {
        const float x_178 = gl_FragCoord.y;
        if ((int(x_178) < 120)) {
          const int x_185 = data[3];
          grey = (0.5f + (float(x_185) / 10.0f));
        } else {
          const float x_190 = gl_FragCoord.y;
          if ((int(x_190) < 150)) {
            discard;
          } else {
            const float x_197 = gl_FragCoord.y;
            if ((int(x_197) < 180)) {
              const int x_204 = data[5];
              grey = (0.5f + (float(x_204) / 10.0f));
            } else {
              const float x_209 = gl_FragCoord.y;
              if ((int(x_209) < 210)) {
                const int x_216 = data[6];
                grey = (0.5f + (float(x_216) / 10.0f));
              } else {
                const float x_221 = gl_FragCoord.y;
                if ((int(x_221) < 240)) {
                  const int x_228 = data[7];
                  grey = (0.5f + (float(x_228) / 10.0f));
                } else {
                  const float x_233 = gl_FragCoord.y;
                  if ((int(x_233) < 270)) {
                    const int x_240 = data[8];
                    grey = (0.5f + (float(x_240) / 10.0f));
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
  const float x_244 = grey;
  const float3 x_245 = float3(x_244, x_244, x_244);
  x_GLF_color = float4(x_245.x, x_245.y, x_245.z, 1.0f);
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
