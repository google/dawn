uint tint_pack2x16float(float2 param_0) {
  uint2 i = f32tof16(param_0);
  return i.x | (i.y << 16);
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void pack2x16float_0e97b3() {
  float2 arg_0 = (1.0f).xx;
  uint res = tint_pack2x16float(arg_0);
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  pack2x16float_0e97b3();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  pack2x16float_0e97b3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  pack2x16float_0e97b3();
  return;
}
