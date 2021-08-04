static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 indexable[16] = (float4[16])0;
  int x_65 = 0;
  int x_65_phi = 0;
  int x_68_phi = 0;
  const float4 x_51 = gl_FragCoord;
  const float2 x_54 = asfloat(x_6[0].xy);
  const float2 x_57 = floor(((float2(x_51.x, x_51.y) / x_54) * 8.0f));
  const int x_63 = ((int(x_57.x) * 8) + int(x_57.y));
  x_65_phi = 0;
  x_68_phi = x_63;
  while (true) {
    int x_79 = 0;
    int x_80 = 0;
    int x_69_phi = 0;
    x_65 = x_65_phi;
    const int x_68 = x_68_phi;
    if ((x_68 > 1)) {
    } else {
      break;
    }
    if (((x_68 & 1) == 1)) {
      x_79 = ((3 * x_68) + 1);
      x_69_phi = x_79;
    } else {
      x_80 = (x_68 / 2);
      x_69_phi = x_80;
    }
    const int x_69 = x_69_phi;
    {
      x_65_phi = asint((x_65 + asint(1)));
      x_68_phi = x_69;
    }
  }
  const float4 tint_symbol_4[16] = {float4(0.0f, 0.0f, 0.0f, 1.0f), float4(0.5f, 0.0f, 0.0f, 1.0f), float4(0.0f, 0.5f, 0.0f, 1.0f), float4(0.5f, 0.5f, 0.0f, 1.0f), float4(0.0f, 0.0f, 0.5f, 1.0f), float4(0.5f, 0.0f, 0.5f, 1.0f), float4(0.0f, 0.5f, 0.5f, 1.0f), float4(0.5f, 0.5f, 0.5f, 1.0f), float4(0.0f, 0.0f, 0.0f, 1.0f), float4(1.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(1.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 1.0f, 1.0f), float4(1.0f, 0.0f, 1.0f, 1.0f), float4(0.0f, 1.0f, 1.0f, 1.0f), float4(1.0f, 1.0f, 1.0f, 1.0f)};
  indexable = tint_symbol_4;
  const float4 x_83 = indexable[asint((x_65 % 16))];
  x_GLF_color = x_83;
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
