static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

int collatz_i1_(inout int v) {
  int count = 0;
  count = 0;
  while (true) {
    const int x_89 = v;
    if ((x_89 > 1)) {
    } else {
      break;
    }
    const int x_92 = v;
    if (((x_92 & 1) == 1)) {
      const int x_98 = v;
      v = ((3 * x_98) + 1);
    } else {
      const int x_101 = v;
      v = (x_101 / 2);
    }
    count = (count + 1);
  }
  return count;
}

void main_1() {
  float2 lin = float2(0.0f, 0.0f);
  int v_1 = 0;
  int param = 0;
  float4 indexable[16] = (float4[16])0;
  const float4 x_63 = gl_FragCoord;
  const float2 x_66 = asfloat(x_10[0].xy);
  lin = (float2(x_63.x, x_63.y) / x_66);
  lin = floor((lin * 8.0f));
  const float x_72 = lin.x;
  const float x_76 = lin.y;
  v_1 = ((int(x_72) * 8) + int(x_76));
  param = v_1;
  const int x_80 = collatz_i1_(param);
  const float4 tint_symbol_4[16] = {float4(0.0f, 0.0f, 0.0f, 1.0f), float4(0.5f, 0.0f, 0.0f, 1.0f), float4(0.0f, 0.5f, 0.0f, 1.0f), float4(0.5f, 0.5f, 0.0f, 1.0f), float4(0.0f, 0.0f, 0.5f, 1.0f), float4(0.5f, 0.0f, 0.5f, 1.0f), float4(0.0f, 0.5f, 0.5f, 1.0f), float4(0.5f, 0.5f, 0.5f, 1.0f), float4(0.0f, 0.0f, 0.0f, 1.0f), float4(1.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(1.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 1.0f, 1.0f), float4(1.0f, 0.0f, 1.0f, 1.0f), float4(0.0f, 1.0f, 1.0f, 1.0f), float4(1.0f, 1.0f, 1.0f, 1.0f)};
  indexable = tint_symbol_4;
  const float4 x_83 = indexable[(x_80 % 16)];
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
