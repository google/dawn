cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[1];
};
static float4 gv = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int temp = 0;
  const float x_39 = asfloat(x_5[0].x);
  if ((x_39 > 2.0f)) {
    temp = int(max(lerp(float4(1.0f, 1.0f, 1.0f, 1.0f), float4(1.0f, 1.0f, 1.0f, 1.0f), gv), float4(8.600000381f, 8.600000381f, 8.600000381f, 8.600000381f)).y);
    if ((temp < 150)) {
      discard;
    }
    if ((temp < 180)) {
      discard;
    }
  }
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
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
  const main_out tint_symbol_2 = {x_GLF_color};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
