struct tint_symbol {
  float4 value : SV_Target0;
};

float4 main_inner() {
  return float4(0.100000001f, 0.200000003f, 0.300000012f, 0.400000006f);
}

tint_symbol main() {
  const float4 inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
