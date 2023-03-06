int tint_insert_bits(int v, int n, uint offset, uint count) {
  const uint e = (offset + count);
  const uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << offset) : 0) & int(mask)) | (v & int(~(mask))));
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void insertBits_65468b() {
  int arg_0 = 1;
  int arg_1 = 1;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  insertBits_65468b();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  insertBits_65468b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  insertBits_65468b();
  return;
}
