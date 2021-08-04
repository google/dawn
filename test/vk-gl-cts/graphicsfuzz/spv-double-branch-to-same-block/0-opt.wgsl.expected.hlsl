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
  const int x_254 = from;
  k = x_254;
  const int x_255 = from;
  i = x_255;
  const int x_256 = mid;
  j = (x_256 + 1);
  while (true) {
    const int x_262 = i;
    const int x_263 = mid;
    const int x_265 = j;
    const int x_266 = to;
    bool tint_tmp = (x_262 <= x_263);
    if (tint_tmp) {
      tint_tmp = (x_265 <= x_266);
    }
    if ((tint_tmp)) {
    } else {
      break;
    }
    const int x_272 = data[i];
    const int x_275 = data[j];
    if ((x_272 < x_275)) {
      const int x_280 = k;
      k = (x_280 + 1);
      const int x_282 = i;
      i = (x_282 + 1);
      const int x_285 = data[x_282];
      temp[x_280] = x_285;
    } else {
      const int x_287 = k;
      k = (x_287 + 1);
      const int x_289 = j;
      j = (x_289 + 1);
      const int x_292 = data[x_289];
      temp[x_287] = x_292;
    }
  }
  while (true) {
    const int x_298 = i;
    const int x_300 = i;
    const int x_301 = mid;
    bool tint_tmp_1 = (x_298 < 10);
    if (tint_tmp_1) {
      tint_tmp_1 = (x_300 <= x_301);
    }
    if ((tint_tmp_1)) {
    } else {
      break;
    }
    const int x_305 = k;
    k = (x_305 + 1);
    const int x_307 = i;
    i = (x_307 + 1);
    const int x_310 = data[x_307];
    temp[x_305] = x_310;
  }
  const int x_312 = from;
  i_1 = x_312;
  while (true) {
    const int x_317 = i_1;
    const int x_318 = to;
    if ((x_317 <= x_318)) {
    } else {
      break;
    }
    const int x_321 = i_1;
    const int x_324 = temp[i_1];
    data[x_321] = x_324;
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
  const float x_87 = asfloat(x_28[0].x);
  i_3 = int(x_87);
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
      const int x_136 = j_1;
      const int x_139 = data[j_1];
      temp[x_136] = x_139;
    }
  }
  mergeSort_();
  const float x_145 = gl_FragCoord.y;
  if ((int(x_145) < 30)) {
    const int x_152 = data[0];
    grey = (0.5f + (float(x_152) / 10.0f));
  } else {
    const float x_157 = gl_FragCoord.y;
    if ((int(x_157) < 60)) {
      const int x_164 = data[1];
      grey = (0.5f + (float(x_164) / 10.0f));
    } else {
      const float x_169 = gl_FragCoord.y;
      if ((int(x_169) < 90)) {
        const int x_176 = data[2];
        grey = (0.5f + (float(x_176) / 10.0f));
      } else {
        const float x_181 = gl_FragCoord.y;
        if ((int(x_181) < 120)) {
          const int x_188 = data[3];
          grey = (0.5f + (float(x_188) / 10.0f));
        } else {
          const float x_193 = gl_FragCoord.y;
          if ((int(x_193) < 150)) {
            discard;
          } else {
            const float x_200 = gl_FragCoord.y;
            if ((int(x_200) < 180)) {
              const int x_207 = data[5];
              grey = (0.5f + (float(x_207) / 10.0f));
            } else {
              const float x_212 = gl_FragCoord.y;
              if ((int(x_212) < 210)) {
                const int x_219 = data[6];
                grey = (0.5f + (float(x_219) / 10.0f));
              } else {
                const float x_224 = gl_FragCoord.y;
                if ((int(x_224) < 240)) {
                  const int x_231 = data[7];
                  grey = (0.5f + (float(x_231) / 10.0f));
                } else {
                  const float x_236 = gl_FragCoord.y;
                  if ((int(x_236) < 270)) {
                    const int x_243 = data[8];
                    grey = (0.5f + (float(x_243) / 10.0f));
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
  const float x_247 = grey;
  const float3 x_248 = float3(x_247, x_247, x_247);
  x_GLF_color = float4(x_248.x, x_248.y, x_248.z, 1.0f);
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
