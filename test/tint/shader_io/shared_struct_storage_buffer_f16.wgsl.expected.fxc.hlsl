SKIP: FAILED

struct S {
  float f;
  uint u;
  float4 v;
  float16_t x;
  vector<float16_t, 3> y;
};

RWByteAddressBuffer output : register(u0, space0);

struct tint_symbol_1 {
  float f : TEXCOORD0;
  nointerpolation uint u : TEXCOORD1;
  float16_t x : TEXCOORD2;
  vector<float16_t, 3> y : TEXCOORD3;
  float4 v : SV_Position;
};

void tint_symbol_2(RWByteAddressBuffer buffer, uint offset, S value) {
  buffer.Store((offset + 0u), asuint(value.f));
  buffer.Store((offset + 4u), asuint(value.u));
  buffer.Store4((offset + 128u), asuint(value.v));
  buffer.Store<float16_t>((offset + 160u), value.x);
  buffer.Store<vector<float16_t, 3> >((offset + 192u), value.y);
}

void frag_main_inner(S input) {
  const float f = input.f;
  const uint u = input.u;
  const float4 v = input.v;
  const float16_t x = input.x;
  const vector<float16_t, 3> y = input.y;
  tint_symbol_2(output, 0u, input);
}

void frag_main(tint_symbol_1 tint_symbol) {
  const S tint_symbol_8 = {tint_symbol.f, tint_symbol.u, tint_symbol.v, tint_symbol.x, tint_symbol.y};
  frag_main_inner(tint_symbol_8);
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\shader_io\Shader@0x00000223CE4E5B80(5,3-11): error X3000: unrecognized identifier 'float16_t'

