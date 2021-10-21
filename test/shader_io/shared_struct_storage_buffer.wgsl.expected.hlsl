struct S {
  float f;
  uint u;
  float4 v;
};

RWByteAddressBuffer output : register(u0, space0);

struct tint_symbol_1 {
  float f : TEXCOORD0;
  nointerpolation uint u : TEXCOORD1;
  float4 v : SV_Position;
};

void tint_symbol_2(RWByteAddressBuffer buffer, uint offset, S value) {
  buffer.Store((offset + 0u), asuint(value.f));
  buffer.Store((offset + 4u), asuint(value.u));
  buffer.Store4((offset + 128u), asuint(value.v));
}

void frag_main_inner(S input) {
  const float f = input.f;
  const uint u = input.u;
  const float4 v = input.v;
  tint_symbol_2(output, 0u, input);
}

void frag_main(tint_symbol_1 tint_symbol) {
  const S tint_symbol_6 = {tint_symbol.f, tint_symbol.u, tint_symbol.v};
  frag_main_inner(tint_symbol_6);
  return;
}
