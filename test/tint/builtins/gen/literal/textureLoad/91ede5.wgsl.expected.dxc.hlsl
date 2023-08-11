RWTexture3D<float4> arg_0 : register(u0, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureLoad_91ede5() {
  float4 res = arg_0.Load(uint4((1u).xxx, uint(0)));
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureLoad_91ede5();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureLoad_91ede5();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_91ede5();
  return;
}
