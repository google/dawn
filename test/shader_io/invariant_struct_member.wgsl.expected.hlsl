struct Out {
  float4 pos;
};
struct tint_symbol {
  precise float4 pos : SV_Position;
};

tint_symbol main() {
  const Out tint_symbol_1 = (Out)0;
  const tint_symbol tint_symbol_2 = {tint_symbol_1.pos};
  return tint_symbol_2;
}
