cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float3 v = float3(0.0f, 0.0f, 0.0f);
  float d = 0.0f;
  const float x_36 = asfloat(x_6[0].x);
  v = lerp(float3(5.0f, 8.0f, -12.199999809f), float3(1.0f, 4.900000095f, -2.099999905f), float3(x_36, x_36, x_36));
  d = distance(v, float3(1.0f, 4.900000095f, -2.099999905f));
  if ((d < 0.100000001f)) {
    const float x_47 = v.x;
    x_GLF_color = float4(x_47, 0.0f, 0.0f, 1.0f);
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
