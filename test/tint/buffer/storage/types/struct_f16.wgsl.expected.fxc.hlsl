SKIP: FAILED

struct Inner {
  float16_t scalar_f16;
  vector<float16_t, 3> vec3_f16;
  matrix<float16_t, 2, 4> mat2x4_f16;
};
struct S {
  Inner inner;
};

ByteAddressBuffer tint_symbol : register(t0, space0);
RWByteAddressBuffer tint_symbol_1 : register(u1, space0);

matrix<float16_t, 2, 4> tint_symbol_6(ByteAddressBuffer buffer, uint offset) {
  return matrix<float16_t, 2, 4>(buffer.Load<vector<float16_t, 4> >((offset + 0u)), buffer.Load<vector<float16_t, 4> >((offset + 8u)));
}

Inner tint_symbol_3(ByteAddressBuffer buffer, uint offset) {
  const Inner tint_symbol_14 = {buffer.Load<float16_t>((offset + 0u)), buffer.Load<vector<float16_t, 3> >((offset + 8u)), tint_symbol_6(buffer, (offset + 16u))};
  return tint_symbol_14;
}

S tint_symbol_2(ByteAddressBuffer buffer, uint offset) {
  const S tint_symbol_15 = {tint_symbol_3(buffer, (offset + 0u))};
  return tint_symbol_15;
}

void tint_symbol_12(RWByteAddressBuffer buffer, uint offset, matrix<float16_t, 2, 4> value) {
  buffer.Store<vector<float16_t, 4> >((offset + 0u), value[0u]);
  buffer.Store<vector<float16_t, 4> >((offset + 8u), value[1u]);
}

void tint_symbol_9(RWByteAddressBuffer buffer, uint offset, Inner value) {
  buffer.Store<float16_t>((offset + 0u), value.scalar_f16);
  buffer.Store<vector<float16_t, 3> >((offset + 8u), value.vec3_f16);
  tint_symbol_12(buffer, (offset + 16u), value.mat2x4_f16);
}

void tint_symbol_8(RWByteAddressBuffer buffer, uint offset, S value) {
  tint_symbol_9(buffer, (offset + 0u), value.inner);
}

[numthreads(1, 1, 1)]
void main() {
  const S t = tint_symbol_2(tint_symbol, 0u);
  tint_symbol_8(tint_symbol_1, 0u, t);
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\buffer\Shader@0x000001A50B947690(2,3-11): error X3000: unrecognized identifier 'float16_t'

