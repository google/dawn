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
float4 textureLoad_46a93f() {
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  uint v_1 = min(uint(int(1)), (v.z - 1u));
  uint4 v_2 = (0u).xxxx;
  arg_0.GetDimensions(0u, v_2.x, v_2.y, v_2.z, v_2.w);
  uint4 v_3 = (0u).xxxx;
  arg_0.GetDimensions(uint(min(1u, (v_2.w - 1u))), v_3.x, v_3.y, v_3.z, v_3.w);
  int2 v_4 = int2(min((1u).xx, (v_3.xy - (1u).xx)));
  int v_5 = int(v_1);
  float4 res = float4(arg_0.Load(int4(v_4, v_5, int(min(1u, (v_2.w - 1u))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_46a93f()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_46a93f()));
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = textureLoad_46a93f();
  VertexOutput v_6 = tint_symbol;
  return v_6;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_7 = vertex_main_inner();
  vertex_main_outputs v_8 = {v_7.prevent_dce, v_7.pos};
  return v_8;
}

