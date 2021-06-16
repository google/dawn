struct S {
  /* 0x0000 */ float f;
  /* 0x0004 */ uint u;
  /* 0x0008 */ int tint_pad_0[30];
  /* 0x0080 */ float4 v;
  /* 0x0090 */ int tint_pad_1[28];
};

S make_S(float param_0,
         uint param_1,
         float4 param_2) {
  S output;
  output.f = param_0;
  output.u = param_1;
  output.v = param_2;
  return output;
}

void tint_symbol_5(RWByteAddressBuffer buffer, uint offset, S value) {
  buffer.Store((offset + 0u), asuint(value.f));
  buffer.Store((offset + 4u), asuint(value.u));
  buffer.Store4((offset + 128u), asuint(value.v));
}

RWByteAddressBuffer output : register(u0, space0);

struct tint_symbol_1 {
  float f : TEXCOORD0;
  uint u : TEXCOORD1;
  float4 v : SV_Position;
};

void frag_main(tint_symbol_1 tint_symbol) {
  const S input = make_S(tint_symbol.f, tint_symbol.u, tint_symbol.v);
  const float f = input.f;
  const uint u = input.u;
  const float4 v = input.v;
  tint_symbol_5(output, 0u, input);
  return;
}
