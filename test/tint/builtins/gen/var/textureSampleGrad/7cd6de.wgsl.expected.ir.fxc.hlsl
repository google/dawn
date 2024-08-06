struct VertexOutput {
  float4 pos;
  float4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float4 textureSampleGrad_7cd6de() {
  float2 arg_2 = (1.0f).xx;
  uint arg_3 = 1u;
  float2 arg_4 = (1.0f).xx;
  float2 arg_5 = (1.0f).xx;
  Texture2DArray<float4> v = arg_0;
  SamplerState v_1 = arg_1;
  float2 v_2 = arg_2;
  float2 v_3 = arg_4;
  float2 v_4 = arg_5;
  float4 res = v.SampleGrad(v_1, float3(v_2, float(arg_3)), v_3, v_4, (1).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSampleGrad_7cd6de()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureSampleGrad_7cd6de()));
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = textureSampleGrad_7cd6de();
  VertexOutput v_5 = tint_symbol;
  return v_5;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_6 = vertex_main_inner();
  VertexOutput v_7 = v_6;
  VertexOutput v_8 = v_6;
  vertex_main_outputs v_9 = {v_8.prevent_dce, v_7.pos};
  return v_9;
}

