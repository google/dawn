struct tint_symbol {
  float4 value : SV_Target0;
};

void bar() {
}

tint_symbol main() {
  float2 a = float2(0.0f, 0.0f);
  bar();
  const tint_symbol tint_symbol_1 = {float4(0.400000006f, 0.400000006f, 0.800000012f, 1.0f)};
  return tint_symbol_1;
}

