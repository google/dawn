static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 indexable[16] = (float4[16])0;
  int2 x_80 = int2(0, 0);
  int2 x_113 = int2(0, 0);
  int x_119 = 0;
  int2 x_80_phi = int2(0, 0);
  int x_83_phi = 0;
  int2 x_114_phi = int2(0, 0);
  int2 x_116_phi = int2(0, 0);
  const float4 x_58 = gl_FragCoord;
  const float2 x_61 = asfloat(x_6[0].xy);
  const float2 x_62 = (float2(x_58.x, x_58.y) / x_61);
  const int x_65 = int((x_62.x * 8.0f));
  const int x_69 = int((x_62.y * 8.0f));
  const int2 x_78 = int2(((((x_65 & 5) | (x_69 & 10)) * 8) + ((x_69 & 5) | (x_65 & 10))), 0);
  x_80_phi = x_78;
  x_83_phi = 0;
  while (true) {
    int2 x_94 = int2(0, 0);
    int2 x_102 = int2(0, 0);
    int x_84 = 0;
    int2 x_95_phi = int2(0, 0);
    int2 x_103_phi = int2(0, 0);
    x_80 = x_80_phi;
    const int x_83 = x_83_phi;
    if ((x_83 < 100)) {
    } else {
      break;
    }
    x_95_phi = x_80;
    if ((x_80.x > 0)) {
      x_94 = x_80;
      x_94.y = (x_80.y - 1);
      x_95_phi = x_94;
    }
    const int2 x_95 = x_95_phi;
    x_103_phi = x_95;
    if ((x_95.x < 0)) {
      x_102 = x_95;
      x_102.y = (x_95.y + 1);
      x_103_phi = x_102;
    }
    const int2 x_103 = x_103_phi;
    int2 x_81_1 = x_103;
    x_81_1.x = (x_103.x + (x_103.y / 2));
    const int2 x_81 = x_81_1;
    {
      x_84 = (x_83 + 1);
      x_80_phi = x_81;
      x_83_phi = x_84;
    }
  }
  const int x_108 = x_80.x;
  x_114_phi = x_80;
  if ((x_108 < 0)) {
    x_113 = x_80;
    x_113.x = -(x_108);
    x_114_phi = x_113;
  }
  x_116_phi = x_114_phi;
  while (true) {
    int2 x_117 = int2(0, 0);
    const int2 x_116 = x_116_phi;
    x_119 = x_116.x;
    if ((x_119 > 15)) {
    } else {
      break;
    }
    {
      x_117 = x_116;
      x_117.x = asint((x_119 - asint(16)));
      x_116_phi = x_117;
    }
  }
  const float4 tint_symbol_4[16] = {float4(0.0f, 0.0f, 0.0f, 1.0f), float4(0.5f, 0.0f, 0.0f, 1.0f), float4(0.0f, 0.5f, 0.0f, 1.0f), float4(0.5f, 0.5f, 0.0f, 1.0f), float4(0.0f, 0.0f, 0.5f, 1.0f), float4(0.5f, 0.0f, 0.5f, 1.0f), float4(0.0f, 0.5f, 0.5f, 1.0f), float4(0.5f, 0.5f, 0.5f, 1.0f), float4(0.0f, 0.0f, 0.0f, 1.0f), float4(1.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(1.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 1.0f, 1.0f), float4(1.0f, 0.0f, 1.0f, 1.0f), float4(0.0f, 1.0f, 1.0f, 1.0f), float4(1.0f, 1.0f, 1.0f, 1.0f)};
  indexable = tint_symbol_4;
  const float4 x_124[16] = indexable;
  const float4 tint_symbol_5[16] = {float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)};
  indexable = tint_symbol_5;
  indexable = x_124;
  const float4 x_125 = indexable[x_119];
  x_GLF_color = x_125;
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
  const main_out tint_symbol_6 = {x_GLF_color};
  return tint_symbol_6;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
