static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float3 GLF_live6mand_() {
  return lerp(asfloat(uint3(38730u, 63193u, 63173u)), float3(463.0f, 4.0f, 0.0f), float3(2.0f, 2.0f, 2.0f));
}

void main_1() {
  const float3 x_27 = GLF_live6mand_();
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
  const main_out tint_symbol_1 = {x_GLF_color};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
