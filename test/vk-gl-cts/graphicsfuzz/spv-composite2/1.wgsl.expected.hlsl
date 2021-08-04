static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 indexable[16] = (float4[16])0;
  int x_72 = 0;
  int x_72_phi = 0;
  int x_75_phi = 0;
  const float4 x_54 = gl_FragCoord;
  const float2 x_55 = float2(x_54.x, x_54.y);
  const float2 x_58 = asfloat(x_6[0].xy);
  const float2 x_59 = (x_55 / x_58);
  const int x_70 = (int((x_59.x * float4(float4(0.0f, x_55, 0.5f).w, 10.0f, float2(0.0f, 0.0f)).y)) + (int((x_59.y * 10.0f)) * 10));
  x_72_phi = 100;
  x_75_phi = 0;
  while (true) {
    int x_73 = 0;
    int x_76 = 0;
    x_72 = x_72_phi;
    const int x_75 = x_75_phi;
    if ((x_75 < x_70)) {
    } else {
      break;
    }
    {
      x_73 = (((4 * asint(x_72)) * (1000 - asint(x_72))) / 1000);
      x_76 = (x_75 + 1);
      x_72_phi = x_73;
      x_75_phi = x_76;
    }
  }
  const float4 tint_symbol_4[16] = {float4(0.0f, 0.0f, 0.0f, 1.0f), float4(0.5f, 0.0f, 0.0f, 1.0f), float4(0.0f, 0.5f, 0.0f, 1.0f), float4(0.5f, 0.5f, 0.0f, 1.0f), float4(0.0f, 0.0f, 0.5f, 1.0f), float4(0.5f, 0.0f, 0.5f, 1.0f), float4(0.0f, 0.5f, 0.5f, 1.0f), float4(0.5f, 0.5f, 0.5f, 1.0f), float4(0.0f, 0.0f, 0.0f, 1.0f), float4(1.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(1.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 1.0f, 1.0f), float4(1.0f, 0.0f, 1.0f, 1.0f), float4(0.0f, 1.0f, 1.0f, 1.0f), float4(1.0f, 1.0f, 1.0f, 1.0f)};
  indexable = tint_symbol_4;
  const float4 x_84 = indexable[asint((x_72 % 16))];
  x_GLF_color = x_84;
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
