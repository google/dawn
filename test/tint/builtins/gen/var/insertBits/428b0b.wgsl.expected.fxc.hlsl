int3 tint_insert_bits(int3 v, int3 n, uint offset, uint count) {
  const uint e = (offset + count);
  const uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << uint3((offset).xxx)) : (0).xxx) & int3((int(mask)).xxx)) | (v & int3((int(~(mask))).xxx)));
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void insertBits_428b0b() {
  int3 arg_0 = (1).xxx;
  int3 arg_1 = (1).xxx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int3 res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
  prevent_dce.Store3(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  insertBits_428b0b();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  insertBits_428b0b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  insertBits_428b0b();
  return;
}
