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

Interface vert_main_inner() {
  const Interface tint_symbol_3 = {0.40000000596046447754f, 0.60000002384185791016f, (0.0f).xxxx};
  return tint_symbol_3;
}

tint_symbol vert_main() {
  const Interface inner_result = vert_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.col1 = inner_result.col1;
  wrapper_result.col2 = inner_result.col2;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}

struct tint_symbol_2 {
  float col1 : TEXCOORD1;
  float col2 : TEXCOORD2;
  float4 pos : SV_Position;
};

void frag_main_inner(Interface colors) {
  const float r = colors.col1;
  const float g = colors.col2;
}

void frag_main(tint_symbol_2 tint_symbol_1) {
  const Interface tint_symbol_4 = {tint_symbol_1.col1, tint_symbol_1.col2, tint_symbol_1.pos};
  frag_main_inner(tint_symbol_4);
  return;
}
