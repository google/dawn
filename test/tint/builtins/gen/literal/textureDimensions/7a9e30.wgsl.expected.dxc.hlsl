TextureCube<uint4> arg_0 : register(t0, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureDimensions_7a9e30() {
  uint2 tint_tmp;
  arg_0.GetDimensions(tint_tmp.x, tint_tmp.y);
  uint2 res = tint_tmp;
  prevent_dce.Store2(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureDimensions_7a9e30();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureDimensions_7a9e30();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureDimensions_7a9e30();
  return;
}
