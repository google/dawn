struct tint_symbol {
  float4 value : SV_Position;
};

float4 main_inner() {
  float3 light = float3(1.200000048f, 1.0f, 2.0f);
  float3 negative_light = -(light);
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol main() {
  const float4 inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
