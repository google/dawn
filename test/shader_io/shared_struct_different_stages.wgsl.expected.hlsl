struct Interface {
  float col1;
  float col2;
  float4 pos;
};
struct tint_symbol {
  float col1 : TEXCOORD1;
  float col2 : TEXCOORD2;
  float4 pos : SV_Position;
};

tint_symbol vert_main() {
  const Interface tint_symbol_1 = {0.400000006f, 0.600000024f, float4(0.0f, 0.0f, 0.0f, 0.0f)};
  const tint_symbol tint_symbol_4 = {tint_symbol_1.col1, tint_symbol_1.col2, tint_symbol_1.pos};
  return tint_symbol_4;
}

struct tint_symbol_3 {
  float col1 : TEXCOORD1;
  float col2 : TEXCOORD2;
  float4 pos : SV_Position;
};

void frag_main(tint_symbol_3 tint_symbol_2) {
  const Interface colors = {tint_symbol_2.col1, tint_symbol_2.col2, tint_symbol_2.pos};
  const float r = colors.col1;
  const float g = colors.col2;
  return;
}
