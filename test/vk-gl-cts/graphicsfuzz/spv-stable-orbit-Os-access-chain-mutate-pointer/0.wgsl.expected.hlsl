static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 indexable[16] = (float4[16])0;
  int2 x_76 = int2(0, 0);
  int2 x_109 = int2(0, 0);
  int x_115 = 0;
  int2 x_76_phi = int2(0, 0);
  int x_79_phi = 0;
  int2 x_110_phi = int2(0, 0);
  int2 x_112_phi = int2(0, 0);
  const float4 x_55 = gl_FragCoord;
  const float2 x_58 = asfloat(x_6[0].xy);
  const float2 x_59 = (float2(x_55.x, x_55.y) / x_58);
  const int x_62 = int((x_59.x * 8.0f));
  const int x_65 = int((x_59.y * 8.0f));
  const int2 x_74 = int2(((((x_62 & 5) | (x_65 & 10)) * 8) + ((x_65 & 5) | (x_62 & 10))), 0);
  x_76_phi = x_74;
  x_79_phi = 0;
  while (true) {
    int2 x_90 = int2(0, 0);
    int2 x_98 = int2(0, 0);
    int x_80 = 0;
    int2 x_91_phi = int2(0, 0);
    int2 x_99_phi = int2(0, 0);
    x_76 = x_76_phi;
    const int x_79 = x_79_phi;
    if ((x_79 < 100)) {
    } else {
      break;
    }
    x_91_phi = x_76;
    if ((x_76.x > 0)) {
      x_90 = x_76;
      x_90.y = (x_76.y - 1);
      x_91_phi = x_90;
    }
    const int2 x_91 = x_91_phi;
    x_99_phi = x_91;
    if ((x_91.x < 0)) {
      x_98 = x_91;
      x_98.y = (x_91.y + 1);
      x_99_phi = x_98;
    }
    const int2 x_99 = x_99_phi;
    int2 x_77_1 = x_99;
    x_77_1.x = (x_99.x + (x_99.y / 2));
    const int2 x_77 = x_77_1;
    {
      x_80 = (x_79 + 1);
      x_76_phi = x_77;
      x_79_phi = x_80;
    }
  }
  const int x_104 = x_76.x;
  x_110_phi = x_76;
  if ((x_104 < 0)) {
    x_109 = x_76;
    x_109.x = -(x_104);
    x_110_phi = x_109;
  }
  x_112_phi = x_110_phi;
  while (true) {
    int2 x_113 = int2(0, 0);
    const int2 x_112 = x_112_phi;
    x_115 = x_112.x;
    if ((x_115 > 15)) {
    } else {
      break;
    }
    {
      x_113 = x_112;
      x_113.x = asint((x_115 - asint(16)));
      x_112_phi = x_113;
    }
  }
  const float4 tint_symbol_4[16] = {float4(0.0f, 0.0f, 0.0f, 1.0f), float4(0.5f, 0.0f, 0.0f, 1.0f), float4(0.0f, 0.5f, 0.0f, 1.0f), float4(0.5f, 0.5f, 0.0f, 1.0f), float4(0.0f, 0.0f, 0.5f, 1.0f), float4(0.5f, 0.0f, 0.5f, 1.0f), float4(0.0f, 0.5f, 0.5f, 1.0f), float4(0.5f, 0.5f, 0.5f, 1.0f), float4(0.0f, 0.0f, 0.0f, 1.0f), float4(1.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(1.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 1.0f, 1.0f), float4(1.0f, 0.0f, 1.0f, 1.0f), float4(0.0f, 1.0f, 1.0f, 1.0f), float4(1.0f, 1.0f, 1.0f, 1.0f)};
  indexable = tint_symbol_4;
  const float4 x_120 = indexable[x_115];
  x_GLF_color = x_120;
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
