static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 indexable[16] = (float4[16])0;
  int x_71 = 0;
  int x_71_phi = 0;
  int x_74_phi = 0;
  const float4 x_54 = gl_FragCoord;
  const float2 x_55 = float2(x_54.x, x_54.y);
  const float2 x_58 = asfloat(x_6[0].xy);
  const float2 x_61 = ((x_55 / x_58) * 8.0f);
  const float2 x_62 = floor(x_61);
  const int x_69 = ((int(x_62.x) * 8) + int(x_62.y));
  x_71_phi = 0;
  x_74_phi = x_69;
  while (true) {
    int x_85 = 0;
    int x_86 = 0;
    int x_75_phi = 0;
    x_71 = x_71_phi;
    const int x_74 = x_74_phi;
    if ((x_74 > 1)) {
    } else {
      break;
    }
    if (((x_74 & 1) == 1)) {
      x_85 = ((3 * x_74) + 1);
      x_75_phi = x_85;
    } else {
      x_86 = (x_74 / 2);
      x_75_phi = x_86;
    }
    const int x_75 = x_75_phi;
    {
      x_71_phi = asint((x_71 + asint(1)));
      x_74_phi = x_75;
    }
  }
  const float4 tint_symbol_4[16] = {float4(0.0f, 0.0f, 0.0f, 1.0f), float4(0.5f, 0.0f, 0.0f, 1.0f), float4(0.0f, 0.5f, 0.0f, 1.0f), float4(0.5f, 0.5f, 0.0f, 1.0f), float4(0.0f, 0.0f, 0.5f, 1.0f), float4(0.5f, 0.0f, 0.5f, 1.0f), float4(0.0f, 0.5f, 0.5f, 1.0f), float4(0.5f, 0.5f, 0.5f, 1.0f), float4(0.0f, 0.0f, 0.0f, 1.0f), float4(1.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(1.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 1.0f, 1.0f), float4(1.0f, 0.0f, 1.0f, 1.0f), float4(0.0f, 1.0f, 1.0f, 1.0f), float4(1.0f, 1.0f, 1.0f, 1.0f)};
  indexable = tint_symbol_4;
  const float4 x_88[16] = indexable;
  const float4 tint_symbol_5[16] = {float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)};
  indexable = tint_symbol_5;
  indexable = x_88;
  const float4 tint_symbol_6[16] = {float4(0.0f, 0.0f, 0.5f, 1.0f), float4(1.0f, 0.0f, 1.0f, 1.0f), float4(0.0f, 0.5f, 0.5f, 1.0f), float4(0.0f, 1.0f, 1.0f, 1.0f), float4(1.0f, 1.0f, 1.0f, 1.0f), float4(0.0f, 0.5f, 0.5f, 1.0f), float4(0.0f, 0.5f, 0.5f, 1.0f), float4(0.5f, 0.0f, 0.5f, 1.0f), float4(1.0f, 1.0f, 0.0f, 1.0f), x_54, float4(0.0f, 0.0f, 0.5f, 1.0f), float4(0.5f, 0.5f, 0.0f, 1.0f), float4(0.0f, 0.5f, 0.0f, 1.0f), float4(0.5f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(1.0f, 0.0f, 0.0f, 1.0f)};
  const float4 x_89 = tint_symbol_6[1u];
  const float4 x_90[16] = {float4(0.5f, 0.0f, 0.5f, 1.0f), float4(0.5f, 0.0f, 0.5f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 0.0f, 1.0f), float4(0.5f, 0.0f, 0.5f, 1.0f), float4(0.5f, 0.5f, 0.5f, 1.0f), float4(1.0f, 0.0f, 1.0f, 1.0f), float4(0.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, 8.0f, x_55), float4(0.0f, 0.0f, 0.5f, 1.0f), float4(0.0f, 0.5f, 0.5f, 1.0f), float4(0.5f, 0.0f, 0.5f, 1.0f), float4(1.0f, 1.0f, 0.0f, 1.0f), float4(1.0f, 0.0f, 1.0f, 1.0f), float4(x_61, 0.5f, 1.0f)};
  const float4 x_92 = indexable[asint((x_71 % 16))];
  x_GLF_color = x_92;
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
  const main_out tint_symbol_7 = {x_GLF_color};
  return tint_symbol_7;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
