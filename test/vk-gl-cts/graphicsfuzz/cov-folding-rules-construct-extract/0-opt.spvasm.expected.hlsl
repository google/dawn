cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float2 a = float2(0.0f, 0.0f);
  float2 b = float2(0.0f, 0.0f);
  bool x_46 = false;
  bool x_47_phi = false;
  const float2 x_32 = asfloat(x_6[0].xy);
  a = x_32;
  const float x_34 = a.x;
  b = float2(x_34, clamp(a, float2(1.0f, 1.0f), float2(1.0f, 1.0f)).y);
  const float x_40 = b.x;
  const bool x_41 = (x_40 == 2.0f);
  x_47_phi = x_41;
  if (x_41) {
    const float x_45 = b.y;
    x_46 = (x_45 == 1.0f);
    x_47_phi = x_46;
  }
  if (x_47_phi) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
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
