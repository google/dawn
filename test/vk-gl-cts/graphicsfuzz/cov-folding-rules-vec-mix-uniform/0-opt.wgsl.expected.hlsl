cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float2 v = float2(0.0f, 0.0f);
  float d = 0.0f;
  const float2 x_37 = asfloat(x_6[0].xy);
  v = lerp(float2(2.0f, 3.0f), float2(4.0f, 5.0f), x_37);
  d = distance(v, float2(2.0f, 5.0f));
  if ((d < 0.100000001f)) {
    const float x_47 = v.x;
    const float x_50 = v.y;
    x_GLF_color = float4((x_47 - 1.0f), (x_50 - 5.0f), 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  }
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
