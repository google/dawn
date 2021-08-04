static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 indexable[16] = (float4[16])0;
  int2 x_77 = int2(0, 0);
  int2 x_110 = int2(0, 0);
  int x_116 = 0;
  int2 x_77_phi = int2(0, 0);
  int x_80_phi = 0;
  int2 x_111_phi = int2(0, 0);
  int2 x_113_phi = int2(0, 0);
  const float4 x_56 = gl_FragCoord;
  const float2 x_59 = asfloat(x_6[0].xy);
  const float2 x_60 = (float2(x_56.x, x_56.y) / x_59);
  const int x_63 = int((x_60.x * 8.0f));
  const int x_66 = int((x_60.y * 8.0f));
  const int2 x_75 = int2(((((x_63 & 5) | (x_66 & 10)) * 8) + ((x_66 & 5) | (x_63 & 10))), 0);
  x_77_phi = x_75;
  x_80_phi = 0;
  while (true) {
    int2 x_91 = int2(0, 0);
    int2 x_99 = int2(0, 0);
    int x_81 = 0;
    int2 x_92_phi = int2(0, 0);
    int2 x_100_phi = int2(0, 0);
    x_77 = x_77_phi;
    const int x_80 = x_80_phi;
    if ((x_80 < 100)) {
    } else {
      break;
    }
    x_92_phi = x_77;
    if ((x_77.x > 0)) {
      x_91 = x_77;
      x_91.y = (x_77.y - 1);
      x_92_phi = x_91;
    }
    const int2 x_92 = x_92_phi;
    x_100_phi = x_92;
    if ((x_92.x < 0)) {
      x_99 = x_92;
      x_99.y = (x_92.y + 1);
      x_100_phi = x_99;
    }
    const int2 x_100 = x_100_phi;
    int2 x_78_1 = x_100;
    x_78_1.x = (x_100.x + (x_100.y / 2));
    const int2 x_78 = x_78_1;
    {
      x_81 = (x_80 + 1);
      x_77_phi = x_78;
      x_80_phi = x_81;
    }
  }
  const int x_105 = x_77.x;
  x_111_phi = x_77;
  if ((x_105 < 0)) {
    x_110 = int2(0, 0);
    x_110.x = -(x_105);
    x_111_phi = x_110;
  }
  x_113_phi = x_111_phi;
  while (true) {
    int2 x_114 = int2(0, 0);
    x_116 = x_113_phi.x;
    if ((x_116 > 15)) {
    } else {
      break;
    }
    {
      x_114 = int2(0, 0);
      x_114.x = asint((x_116 - asint(16)));
      x_113_phi = x_114;
    }
  }
  const float4 tint_symbol_4[16] = {float4(0.0f, 0.0f, 0.0f, 1.0f), float4(0.5f, 0.0f, 0.0f, 1.0f), float4(0.0f, 0.5f, 0.0f, 1.0f), float4(0.5f, 0.5f, 0.0f, 1.0f), float4(0.0f, 0.0f, 0.5f, 1.0f), float4(0.5f, 0.0f, 0.5f, 1.0f), float4(0.0f, 0.5f, 0.5f, 1.0f), float4(0.5f, 0.5f, 0.5f, 1.0f), float4(0.0f, 0.0f, 0.0f, 1.0f), float4(1.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(1.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 1.0f, 1.0f), float4(1.0f, 0.0f, 1.0f, 1.0f), float4(0.0f, 1.0f, 1.0f, 1.0f), float4(1.0f, 1.0f, 1.0f, 1.0f)};
  indexable = tint_symbol_4;
  const float4 x_121 = indexable[x_116];
  x_GLF_color = x_121;
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
