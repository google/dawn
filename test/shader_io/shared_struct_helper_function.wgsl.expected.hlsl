struct VertexOutput {
  float4 pos;
  int loc0;
};

VertexOutput foo(float x) {
  const VertexOutput tint_symbol_4 = {float4(x, x, x, 1.0f), 42};
  return tint_symbol_4;
}

struct tint_symbol {
  int loc0 : TEXCOORD0;
  float4 pos : SV_Position;
};

tint_symbol vert_main1() {
  const VertexOutput tint_symbol_1 = foo(0.5f);
  const tint_symbol tint_symbol_5 = {tint_symbol_1.loc0, tint_symbol_1.pos};
  return tint_symbol_5;
}

struct tint_symbol_2 {
  int loc0 : TEXCOORD0;
  float4 pos : SV_Position;
};

tint_symbol_2 vert_main2() {
  const VertexOutput tint_symbol_3 = foo(0.25f);
  const tint_symbol_2 tint_symbol_6 = {tint_symbol_3.loc0, tint_symbol_3.pos};
  return tint_symbol_6;
}
