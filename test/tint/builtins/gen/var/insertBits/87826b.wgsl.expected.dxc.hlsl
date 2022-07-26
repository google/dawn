uint3 tint_insert_bits(uint3 v, uint3 n, uint offset, uint count) {
  const uint s = min(offset, 32u);
  const uint e = min(32u, (s + count));
  const uint mask = (((1u << s) - 1u) ^ ((1u << e) - 1u));
  return (((n << uint3((s).xxx)) & uint3((mask).xxx)) | (v & uint3((~(mask)).xxx)));
}

void insertBits_87826b() {
  uint3 arg_0 = (1u).xxx;
  uint3 arg_1 = (1u).xxx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint3 res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  insertBits_87826b();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  insertBits_87826b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  insertBits_87826b();
  return;
}
