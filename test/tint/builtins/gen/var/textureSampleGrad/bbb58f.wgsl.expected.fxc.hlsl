TextureCubeArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureSampleGrad_bbb58f() {
  float3 arg_2 = (1.0f).xxx;
  uint arg_3 = 1u;
  float3 arg_4 = (1.0f).xxx;
  float3 arg_5 = (1.0f).xxx;
  float4 res = arg_0.SampleGrad(arg_1, float4(arg_2, float(arg_3)), arg_4, arg_5);
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureSampleGrad_bbb58f();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureSampleGrad_bbb58f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleGrad_bbb58f();
  return;
}
