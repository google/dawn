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
  const int x_255 = from;
  k = x_255;
  const int x_256 = from;
  i = x_256;
  const int x_257 = mid;
  j = (x_257 + 1);
  while (true) {
    const int x_263 = i;
    const int x_264 = mid;
    const int x_266 = j;
    const int x_267 = to;
    if (((x_263 <= x_264) & (x_266 <= x_267))) {
    } else {
      break;
    }
    const int x_273 = data[i];
    const int x_276 = data[j];
    if ((x_273 < x_276)) {
      const int x_281 = k;
      k = (x_281 + 1);
      const int x_283 = i;
      i = (x_283 + 1);
      const int x_286 = data[x_283];
      temp[x_281] = x_286;
    } else {
      const int x_288 = k;
      k = (x_288 + 1);
      const int x_290 = j;
      j = (x_290 + 1);
      const int x_293 = data[x_290];
      temp[x_288] = x_293;
    }
  }
  while (true) {
    const int x_299 = i;
    const int x_301 = i;
    const int x_302 = mid;
    if (((x_299 < 10) & (x_301 <= x_302))) {
    } else {
      break;
    }
    const int x_306 = k;
    k = (x_306 + 1);
    const int x_308 = i;
    i = (x_308 + 1);
    const int x_311 = data[x_308];
    temp[x_306] = x_311;
  }
  const int x_313 = from;
  i_1 = x_313;
  while (true) {
    const int x_318 = i_1;
    const int x_319 = to;
    if ((x_318 <= x_319)) {
    } else {
      break;
    }
    const int x_322 = i_1;
    const int x_325 = temp[i_1];
    data[x_322] = x_325;
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
  const float x_88 = asfloat(x_28[0].x);
  i_3 = int(x_88);
  while (true) {
    switch(i_3) {
      case 9: {
        data[i_3] = -5;
        if (true) {
        } else {
          {
            if ((i_3 < 10)) {
            } else {
              break;
            }
          }
          continue;
        }
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
      const int x_137 = j_1;
      const int x_140 = data[j_1];
      temp[x_137] = x_140;
    }
  }
  mergeSort_();
  const float x_146 = gl_FragCoord.y;
  if ((int(x_146) < 30)) {
    const int x_153 = data[0];
    grey = (0.5f + (float(x_153) / 10.0f));
  } else {
    const float x_158 = gl_FragCoord.y;
    if ((int(x_158) < 60)) {
      const int x_165 = data[1];
      grey = (0.5f + (float(x_165) / 10.0f));
    } else {
      const float x_170 = gl_FragCoord.y;
      if ((int(x_170) < 90)) {
        const int x_177 = data[2];
        grey = (0.5f + (float(x_177) / 10.0f));
      } else {
        const float x_182 = gl_FragCoord.y;
        if ((int(x_182) < 120)) {
          const int x_189 = data[3];
          grey = (0.5f + (float(x_189) / 10.0f));
        } else {
          const float x_194 = gl_FragCoord.y;
          if ((int(x_194) < 150)) {
            discard;
          } else {
            const float x_201 = gl_FragCoord.y;
            if ((int(x_201) < 180)) {
              const int x_208 = data[5];
              grey = (0.5f + (float(x_208) / 10.0f));
            } else {
              const float x_213 = gl_FragCoord.y;
              if ((int(x_213) < 210)) {
                const int x_220 = data[6];
                grey = (0.5f + (float(x_220) / 10.0f));
              } else {
                const float x_225 = gl_FragCoord.y;
                if ((int(x_225) < 240)) {
                  const int x_232 = data[7];
                  grey = (0.5f + (float(x_232) / 10.0f));
                } else {
                  const float x_237 = gl_FragCoord.y;
                  if ((int(x_237) < 270)) {
                    const int x_244 = data[8];
                    grey = (0.5f + (float(x_244) / 10.0f));
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
  const float x_248 = grey;
  const float3 x_249 = float3(x_248, x_248, x_248);
  x_GLF_color = float4(x_249.x, x_249.y, x_249.z, 1.0f);
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
