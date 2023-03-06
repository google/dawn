int2 tint_insert_bits(int2 v, int2 n, uint offset, uint count) {
  const uint e = (offset + count);
  const uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << uint2((offset).xx)) : (0).xx) & int2((int(mask)).xx)) | (v & int2((int(~(mask))).xx)));
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void insertBits_fe6ba6() {
  int2 arg_0 = (1).xx;
  int2 arg_1 = (1).xx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int2 res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
  prevent_dce.Store2(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  insertBits_fe6ba6();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  insertBits_fe6ba6();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  insertBits_fe6ba6();
  return;
}
