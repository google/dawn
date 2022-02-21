void bar() {
}

struct tint_symbol {
  float4 value : SV_Target0;
};

float4 main_inner() {
  float2 a = float2(0.0f, 0.0f);
  bar();
  return float4(0.400000006f, 0.400000006f, 0.800000012f, 1.0f);
}

tint_symbol main() {
  const float4 inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
