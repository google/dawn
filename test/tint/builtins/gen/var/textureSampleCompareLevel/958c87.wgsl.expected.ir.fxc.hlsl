struct VertexOutput {
  float4 pos;
  float prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
TextureCubeArray arg_0 : register(t0, space1);
SamplerComparisonState arg_1 : register(s1, space1);
float textureSampleCompareLevel_958c87() {
  float3 arg_2 = (1.0f).xxx;
  uint arg_3 = 1u;
  float arg_4 = 1.0f;
  TextureCubeArray v = arg_0;
  SamplerComparisonState v_1 = arg_1;
  float3 v_2 = arg_2;
  float v_3 = arg_4;
  float res = v.SampleCmpLevelZero(v_1, float4(v_2, float(arg_3)), v_3);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureSampleCompareLevel_958c87()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(textureSampleCompareLevel_958c87()));
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = textureSampleCompareLevel_958c87();
  VertexOutput v_4 = tint_symbol;
  return v_4;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_5 = vertex_main_inner();
  VertexOutput v_6 = v_5;
  VertexOutput v_7 = v_5;
  vertex_main_outputs v_8 = {v_7.prevent_dce, v_6.pos};
  return v_8;
}

