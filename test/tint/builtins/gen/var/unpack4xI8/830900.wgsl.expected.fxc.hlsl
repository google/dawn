int4 tint_unpack_4xi8(uint a) {
  uint4 a_vec4u = uint4((a).xxxx);
  int4 a_vec4i = asint((a_vec4u << uint4(24u, 16u, 8u, 0u)));
  return (a_vec4i >> (24u).xxxx);
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void unpack4xI8_830900() {
  uint arg_0 = 1u;
  int4 res = tint_unpack_4xi8(arg_0);
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  unpack4xI8_830900();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  unpack4xI8_830900();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  unpack4xI8_830900();
  return;
}
