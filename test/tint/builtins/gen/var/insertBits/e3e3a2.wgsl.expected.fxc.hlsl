uint tint_insert_bits(uint v, uint n, uint offset, uint count) {
  const uint e = (offset + count);
  const uint mask = ((((offset < 32u) ? (1u << offset) : 0u) - 1u) ^ (((e < 32u) ? (1u << e) : 0u) - 1u));
  return ((((offset < 32u) ? (n << offset) : 0u) & mask) | (v & ~(mask)));
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void insertBits_e3e3a2() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  insertBits_e3e3a2();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  insertBits_e3e3a2();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  insertBits_e3e3a2();
  return;
}
