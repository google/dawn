Texture3D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureSampleGrad_5884dd() {
  float3 arg_2 = (1.0f).xxx;
  float3 arg_3 = (1.0f).xxx;
  float3 arg_4 = (1.0f).xxx;
  float4 res = arg_0.SampleGrad(arg_1, arg_2, arg_3, arg_4, int3((1).xxx));
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureSampleGrad_5884dd();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureSampleGrad_5884dd();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleGrad_5884dd();
  return;
}
