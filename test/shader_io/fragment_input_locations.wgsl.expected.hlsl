struct tint_symbol_1 {
  int loc0 : TEXCOORD0;
  uint loc1 : TEXCOORD1;
  float loc2 : TEXCOORD2;
  float4 loc3 : TEXCOORD3;
};

void main(tint_symbol_1 tint_symbol) {
  const int loc0 = tint_symbol.loc0;
  const uint loc1 = tint_symbol.loc1;
  const float loc2 = tint_symbol.loc2;
  const float4 loc3 = tint_symbol.loc3;
  const int i = loc0;
  const uint u = loc1;
  const float f = loc2;
  const float4 v = loc3;
  return;
}
