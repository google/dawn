struct S {
  float f;
  uint u;
  float4 v;
};

RWByteAddressBuffer output : register(u0);

struct tint_symbol_1 {
  float f : TEXCOORD0;
  nointerpolation uint u : TEXCOORD1;
  float4 v : SV_Position;
};

void output_store(uint offset, S value) {
  output.Store((offset + 0u), asuint(value.f));
  output.Store((offset + 4u), asuint(value.u));
  output.Store4((offset + 128u), asuint(value.v));
}

void frag_main_inner(S input) {
  const float f = input.f;
  const uint u = input.u;
  const float4 v = input.v;
  output_store(0u, input);
}

void frag_main(tint_symbol_1 tint_symbol) {
  const S tint_symbol_2 = {tint_symbol.f, tint_symbol.u, tint_symbol.v};
  frag_main_inner(tint_symbol_2);
  return;
}
