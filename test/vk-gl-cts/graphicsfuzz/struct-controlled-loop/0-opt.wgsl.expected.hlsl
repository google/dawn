struct S {
  int f0;
  bool3 f1;
};

cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  S ll = (S)0;
  float sums[9] = (float[9])0;
  const S tint_symbol_2 = {0, bool3(true, true, true)};
  ll = tint_symbol_2;
  while (true) {
    const S x_12 = ll;
    const float x_45 = asfloat(x_7[0].y);
    if ((x_12.f0 != int(x_45))) {
    } else {
      break;
    }
    sums[0] = 0.0f;
    {
      const S x_13 = ll;
      S x_51_1 = ll;
      x_51_1.f0 = (x_13.f0 + 1);
      ll = x_51_1;
    }
  }
  const float x_53 = sums[0];
  const float2 x_54 = float2(x_53, x_53);
  x_GLF_color = float4(1.0f, x_54.x, x_54.y, 1.0f);
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_3 = {x_GLF_color};
  return tint_symbol_3;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
