static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 x_81[8] = {float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)};
  float4 x_82[8] = (float4[8])0;
  float4 x_83[8] = (float4[8])0;
  float4 x_84[8] = (float4[8])0;
  float4 x_85[16] = (float4[16])0;
  float4 x_95 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_95_phi = float4(0.0f, 0.0f, 0.0f, 0.0f);
  int x_98_phi = 0;
  const float4 tint_symbol_4[8] = {float4(4.0f, 4.0f, 20.0f, 4.0f), float4(4.0f, 4.0f, 4.0f, 20.0f), float4(4.0f, 20.0f, 20.0f, 4.0f), float4(20.0f, 4.0f, 4.0f, 8.0f), float4(8.0f, 6.0f, 4.0f, 2.0f), float4(2.0f, 12.0f, 2.0f, 4.0f), float4(16.0f, 2.0f, 4.0f, 4.0f), float4(12.0f, 22.0f, 4.0f, 4.0f)};
  x_81 = tint_symbol_4;
  const float4 x_86[8] = x_81;
  const float4 x_87 = gl_FragCoord;
  const float2 x_90 = asfloat(x_6[0].xy);
  const float2 x_93 = floor(((float2(x_87.x, x_87.y) / x_90) * 32.0f));
  x_95_phi = float4(0.5f, 0.5f, 1.0f, 1.0f);
  x_98_phi = 0;
  while (true) {
    float4 x_142 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    int x_99 = 0;
    bool x_127_phi = false;
    float4 x_96_phi = float4(0.0f, 0.0f, 0.0f, 0.0f);
    x_95 = x_95_phi;
    const int x_98 = x_98_phi;
    if ((x_98 < 8)) {
    } else {
      break;
    }
    float4 x_104 = float4(0.0f, 0.0f, 0.0f, 0.0f);
    x_82 = x_86;
    x_104 = x_82[x_98];
    switch(0u) {
      default: {
        const float x_107 = x_93.x;
        const float x_108 = x_104.x;
        if ((x_107 < x_108)) {
          x_127_phi = false;
          break;
        }
        const float x_112 = x_93.y;
        const float x_113 = x_104.y;
        if ((x_112 < x_113)) {
          x_127_phi = false;
          break;
        }
        if ((x_107 > (x_108 + x_104.z))) {
          x_127_phi = false;
          break;
        }
        if ((x_112 > (x_113 + x_104.w))) {
          x_127_phi = false;
          break;
        }
        x_127_phi = true;
        break;
      }
    }
    const bool x_127 = x_127_phi;
    x_96_phi = x_95;
    if (x_127) {
      const float4 tint_symbol_5[8] = {float4(4.0f, 4.0f, 20.0f, 4.0f), float4(4.0f, 4.0f, 4.0f, 20.0f), float4(4.0f, 20.0f, 20.0f, 4.0f), float4(20.0f, 4.0f, 4.0f, 8.0f), float4(8.0f, 6.0f, 4.0f, 2.0f), float4(2.0f, 12.0f, 2.0f, 4.0f), float4(16.0f, 2.0f, 4.0f, 4.0f), float4(12.0f, 22.0f, 4.0f, 4.0f)};
      x_83 = tint_symbol_5;
      const float x_131 = x_83[x_98].x;
      const float4 tint_symbol_6[8] = {float4(4.0f, 4.0f, 20.0f, 4.0f), float4(4.0f, 4.0f, 4.0f, 20.0f), float4(4.0f, 20.0f, 20.0f, 4.0f), float4(20.0f, 4.0f, 4.0f, 8.0f), float4(8.0f, 6.0f, 4.0f, 2.0f), float4(2.0f, 12.0f, 2.0f, 4.0f), float4(16.0f, 2.0f, 4.0f, 4.0f), float4(12.0f, 22.0f, 4.0f, 4.0f)};
      x_84 = tint_symbol_6;
      const float x_134 = x_84[x_98].y;
      const float4 tint_symbol_7[16] = {float4(0.0f, 0.0f, 0.0f, 1.0f), float4(0.5f, 0.0f, 0.0f, 1.0f), float4(0.0f, 0.5f, 0.0f, 1.0f), float4(0.5f, 0.5f, 0.0f, 1.0f), float4(0.0f, 0.0f, 0.5f, 1.0f), float4(0.5f, 0.0f, 0.5f, 1.0f), float4(0.0f, 0.5f, 0.5f, 1.0f), float4(0.5f, 0.5f, 0.5f, 1.0f), float4(0.0f, 0.0f, 0.0f, 1.0f), float4(1.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(1.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 1.0f, 1.0f), float4(1.0f, 0.0f, 1.0f, 1.0f), float4(0.0f, 1.0f, 1.0f, 1.0f), float4(1.0f, 1.0f, 1.0f, 1.0f)};
      x_85 = tint_symbol_7;
      x_142 = x_85[((((int(x_131) * int(x_134)) + (x_98 * 9)) + 11) % 16)];
      x_96_phi = x_142;
    }
    const float4 x_96 = x_96_phi;
    {
      x_99 = (x_98 + 1);
      x_95_phi = x_96;
      x_98_phi = x_99;
    }
  }
  x_GLF_color = x_95;
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
