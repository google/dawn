void set_float3(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

void set_float2(inout float2 vec, int idx, float val) {
  vec = (idx.xx == int2(0, 1)) ? val.xx : vec;
}

cbuffer cbuffer_x_25 : register(b0, space0) {
  uint4 x_25[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float3 drawShape_vf2_(inout float2 pos) {
  bool c2 = false;
  bool c3 = false;
  bool c4 = false;
  bool c5 = false;
  bool c6 = false;
  int GLF_live4i = 0;
  int GLF_live4_looplimiter5 = 0;
  float4x2 GLF_live7m42 = float4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float3x3 GLF_live7m33 = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int GLF_live7cols = 0;
  int GLF_live7_looplimiter3 = 0;
  int GLF_live7rows = 0;
  int GLF_live7_looplimiter2 = 0;
  int GLF_live7_looplimiter1 = 0;
  int GLF_live7c = 0;
  int GLF_live7r = 0;
  int GLF_live7_looplimiter0 = 0;
  int GLF_live7sum_index = 0;
  int GLF_live7_looplimiter7 = 0;
  int GLF_live7cols_1 = 0;
  int GLF_live7rows_1 = 0;
  float GLF_live7sums[9] = (float[9])0;
  int GLF_live7c_1 = 0;
  int GLF_live7r_1 = 0;
  int x_180 = 0;
  float3x3 indexable = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  const float x_182 = pos.x;
  c2 = (x_182 > 1.0f);
  if (c2) {
    return float3(1.0f, 1.0f, 1.0f);
  }
  const float x_188 = pos.y;
  c3 = (x_188 < 1.0f);
  if (c3) {
    return float3(1.0f, 1.0f, 1.0f);
  }
  const float x_194 = pos.y;
  c4 = (x_194 > 1.0f);
  if (c4) {
    return float3(1.0f, 1.0f, 1.0f);
  }
  const float x_200 = pos.x;
  c5 = (x_200 < 1.0f);
  if (c5) {
    return float3(1.0f, 1.0f, 1.0f);
  }
  const float x_206 = pos.x;
  c6 = ((x_206 + 1.0f) > 1.0f);
  if (c6) {
    return float3(1.0f, 1.0f, 1.0f);
  }
  GLF_live4i = 0;
  {
    for(; (GLF_live4i < 4); GLF_live4i = (GLF_live4i + 1)) {
      if ((GLF_live4_looplimiter5 >= 7)) {
        break;
      }
      GLF_live4_looplimiter5 = (GLF_live4_looplimiter5 + 1);
      GLF_live7m42 = float4x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f), float2(0.0f, 0.0f), float2(1.0f, 0.0f));
      GLF_live7m33 = float3x3(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f));
      GLF_live7cols = 2;
      {
        for(; (GLF_live7cols < 4); GLF_live7cols = (GLF_live7cols + 1)) {
          if ((GLF_live7_looplimiter3 >= 7)) {
            break;
          }
          GLF_live7_looplimiter3 = (GLF_live7_looplimiter3 + 1);
          GLF_live7rows = 2;
          {
            for(; (GLF_live7rows < 4); GLF_live7rows = (GLF_live7rows + 1)) {
              if ((GLF_live7_looplimiter2 >= 7)) {
                break;
              }
              GLF_live7_looplimiter2 = (GLF_live7_looplimiter2 + 1);
              GLF_live7_looplimiter1 = 0;
              GLF_live7c = 0;
              {
                for(; (GLF_live7c < 3); GLF_live7c = (GLF_live7c + 1)) {
                  if ((GLF_live7_looplimiter1 >= 7)) {
                    break;
                  }
                  GLF_live7_looplimiter1 = (GLF_live7_looplimiter1 + 1);
                  GLF_live7r = 0;
                  {
                    for(; (GLF_live7r < 2); GLF_live7r = (GLF_live7r + 1)) {
                      if ((GLF_live7_looplimiter0 >= 7)) {
                        break;
                      }
                      GLF_live7_looplimiter0 = (GLF_live7_looplimiter0 + 1);
                      bool tint_tmp = (GLF_live7c >= 0);
                      if (tint_tmp) {
                        tint_tmp = (GLF_live7c < 3);
                      }
                      bool tint_tmp_1 = (GLF_live7r >= 0);
                      if (tint_tmp_1) {
                        tint_tmp_1 = (GLF_live7r < 3);
                      }
                      set_float3(GLF_live7m33[((tint_tmp) ? GLF_live7c : 0)], ((tint_tmp_1) ? GLF_live7r : 0), 1.0f);
                      const float x_267 = asfloat(x_25[0].y);
                      if ((0.0f > x_267)) {
                      } else {
                        bool tint_tmp_2 = (GLF_live7c >= 0);
                        if (tint_tmp_2) {
                          tint_tmp_2 = (GLF_live7c < 4);
                        }
                        bool tint_tmp_3 = (GLF_live7r >= 0);
                        if (tint_tmp_3) {
                          tint_tmp_3 = (GLF_live7r < 2);
                        }
                        set_float2(GLF_live7m42[((tint_tmp_2) ? GLF_live7c : 0)], ((tint_tmp_3) ? GLF_live7r : 0), 1.0f);
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      GLF_live7sum_index = 0;
      GLF_live7_looplimiter7 = 0;
      GLF_live7cols_1 = 2;
      {
        for(; (GLF_live7cols_1 < 4); GLF_live7cols_1 = (GLF_live7cols_1 + 1)) {
          if ((GLF_live7_looplimiter7 >= 7)) {
            break;
          }
          GLF_live7_looplimiter7 = (GLF_live7_looplimiter7 + 1);
          GLF_live7rows_1 = 2;
          bool tint_tmp_4 = (GLF_live7sum_index >= 0);
          if (tint_tmp_4) {
            tint_tmp_4 = (GLF_live7sum_index < 9);
          }
          GLF_live7sums[((tint_tmp_4) ? GLF_live7sum_index : 0)] = 0.0f;
          GLF_live7c_1 = 0;
          {
            for(; (GLF_live7c_1 < 1); GLF_live7c_1 = (GLF_live7c_1 + 1)) {
              GLF_live7r_1 = 0;
              {
                for(; (GLF_live7r_1 < GLF_live7rows_1); GLF_live7r_1 = (GLF_live7r_1 + 1)) {
                  bool tint_tmp_5 = (GLF_live7sum_index >= 0);
                  if (tint_tmp_5) {
                    tint_tmp_5 = (GLF_live7sum_index < 9);
                  }
                  const int x_310 = ((tint_tmp_5) ? GLF_live7sum_index : 0);
                  const float3x3 x_312 = transpose(GLF_live7m33);
                  if ((GLF_live7c_1 < 3)) {
                    x_180 = 1;
                  } else {
                    const float x_318 = asfloat(x_25[0].x);
                    x_180 = int(x_318);
                  }
                  const int x_320 = x_180;
                  const int x_93 = GLF_live7r_1;
                  indexable = x_312;
                  const float x_324 = indexable[x_320][((x_93 < 3) ? 1 : 0)];
                  const float x_326 = GLF_live7sums[x_310];
                  GLF_live7sums[x_310] = (x_326 + x_324);
                  bool tint_tmp_6 = (GLF_live7sum_index >= 0);
                  if (tint_tmp_6) {
                    tint_tmp_6 = (GLF_live7sum_index < 9);
                  }
                  const int x_332 = ((tint_tmp_6) ? GLF_live7sum_index : 0);
                  const float x_334 = GLF_live7m42[1][GLF_live7r_1];
                  const float x_336 = GLF_live7sums[x_332];
                  GLF_live7sums[x_332] = (x_336 + x_334);
                }
              }
            }
          }
          GLF_live7sum_index = (GLF_live7sum_index + 1);
        }
      }
    }
  }
  return float3(1.0f, 1.0f, 1.0f);
}

void main_1() {
  float2 position = float2(0.0f, 0.0f);
  float2 param = float2(0.0f, 0.0f);
  float2 param_1 = float2(0.0f, 0.0f);
  int i = 0;
  float2 param_2 = float2(0.0f, 0.0f);
  const float x_161 = asfloat(x_25[0].x);
  if ((x_161 >= 2.0f)) {
    const float4 x_165 = gl_FragCoord;
    position = float2(x_165.x, x_165.y);
    param = position;
    const float3 x_168 = drawShape_vf2_(param);
    param_1 = position;
    const float3 x_170 = drawShape_vf2_(param_1);
    i = 25;
    {
      for(; (i > 0); i = (i - 1)) {
        param_2 = position;
        const float3 x_178 = drawShape_vf2_(param_2);
      }
    }
  }
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
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
