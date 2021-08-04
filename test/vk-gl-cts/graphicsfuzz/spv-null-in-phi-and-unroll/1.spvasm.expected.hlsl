static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 x_77[8] = (float4[8])0;
  float4 x_78[8] = (float4[8])0;
  float4 x_79[8] = (float4[8])0;
  float4 x_80[16] = (float4[16])0;
  float4 x_89 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_89_phi = float4(0.0f, 0.0f, 0.0f, 0.0f);
  int x_92_phi = 0;
  const float4 x_81 = gl_FragCoord;
  const float2 x_84 = asfloat(x_6[0].xy);
  const float2 x_87 = floor(((float2(x_81.x, x_81.y) / x_84) * 32.0f));
  x_89_phi = float4(0.5f, 0.5f, 1.0f, 1.0f);
  x_92_phi = 0;
  while (true) {
    float4 x_136 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    int x_93 = 0;
    bool x_121_phi = false;
    float4 x_90_phi = float4(0.0f, 0.0f, 0.0f, 0.0f);
    x_89 = x_89_phi;
    const int x_92 = x_92_phi;
    if ((x_92 < 8)) {
    } else {
      break;
    }
    float4 x_98 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float4 tint_symbol_4[8] = {float4(4.0f, 4.0f, 20.0f, 4.0f), float4(4.0f, 4.0f, 4.0f, 20.0f), float4(4.0f, 20.0f, 20.0f, 4.0f), float4(20.0f, 4.0f, 4.0f, 8.0f), float4(8.0f, 6.0f, 4.0f, 2.0f), float4(2.0f, 12.0f, 2.0f, 4.0f), float4(16.0f, 2.0f, 4.0f, 4.0f), float4(12.0f, 22.0f, 4.0f, 4.0f)};
    x_77 = tint_symbol_4;
    x_98 = x_77[x_92];
    switch(0u) {
      default: {
        const float x_101 = x_87.x;
        const float x_102 = x_98.x;
        if ((x_101 < x_102)) {
          x_121_phi = false;
          break;
        }
        const float x_106 = x_87.y;
        const float x_107 = x_98.y;
        if ((x_106 < x_107)) {
          x_121_phi = false;
          break;
        }
        if ((x_101 > (x_102 + x_98.z))) {
          x_121_phi = false;
          break;
        }
        if ((x_106 > (x_107 + x_98.w))) {
          x_121_phi = false;
          break;
        }
        x_121_phi = true;
        break;
      }
    }
    const bool x_121 = x_121_phi;
    x_90_phi = x_89;
    if (x_121) {
      const float4 tint_symbol_5[8] = {float4(4.0f, 4.0f, 20.0f, 4.0f), float4(4.0f, 4.0f, 4.0f, 20.0f), float4(4.0f, 20.0f, 20.0f, 4.0f), float4(20.0f, 4.0f, 4.0f, 8.0f), float4(8.0f, 6.0f, 4.0f, 2.0f), float4(2.0f, 12.0f, 2.0f, 4.0f), float4(16.0f, 2.0f, 4.0f, 4.0f), float4(12.0f, 22.0f, 4.0f, 4.0f)};
      x_78 = tint_symbol_5;
      const float x_125 = x_78[x_92].x;
      const float4 tint_symbol_6[8] = {float4(4.0f, 4.0f, 20.0f, 4.0f), float4(4.0f, 4.0f, 4.0f, 20.0f), float4(4.0f, 20.0f, 20.0f, 4.0f), float4(20.0f, 4.0f, 4.0f, 8.0f), float4(8.0f, 6.0f, 4.0f, 2.0f), float4(2.0f, 12.0f, 2.0f, 4.0f), float4(16.0f, 2.0f, 4.0f, 4.0f), float4(12.0f, 22.0f, 4.0f, 4.0f)};
      x_79 = tint_symbol_6;
      const float x_128 = x_79[x_92].y;
      const float4 tint_symbol_7[16] = {float4(0.0f, 0.0f, 0.0f, 1.0f), float4(0.5f, 0.0f, 0.0f, 1.0f), float4(0.0f, 0.5f, 0.0f, 1.0f), float4(0.5f, 0.5f, 0.0f, 1.0f), float4(0.0f, 0.0f, 0.5f, 1.0f), float4(0.5f, 0.0f, 0.5f, 1.0f), float4(0.0f, 0.5f, 0.5f, 1.0f), float4(0.5f, 0.5f, 0.5f, 1.0f), float4(0.0f, 0.0f, 0.0f, 1.0f), float4(1.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(1.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 1.0f, 1.0f), float4(1.0f, 0.0f, 1.0f, 1.0f), float4(0.0f, 1.0f, 1.0f, 1.0f), float4(1.0f, 1.0f, 1.0f, 1.0f)};
      x_80 = tint_symbol_7;
      x_136 = x_80[((((int(x_125) * int(x_128)) + (x_92 * 9)) + 11) % 16)];
      x_90_phi = x_136;
    }
    const float4 x_90 = x_90_phi;
    {
      x_93 = (x_92 + 1);
      x_89_phi = x_90;
      x_92_phi = x_93;
    }
  }
  x_GLF_color = x_89;
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
  const main_out tint_symbol_8 = {x_GLF_color};
  return tint_symbol_8;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
