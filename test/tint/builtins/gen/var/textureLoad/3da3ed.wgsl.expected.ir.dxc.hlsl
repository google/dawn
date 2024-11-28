struct VertexOutput {
  float4 pos;
  float4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
Texture1D<float4> arg_0 : register(t0, space1);
float4 textureLoad_3da3ed() {
  int arg_1 = int(1);
  uint arg_2 = 1u;
  int v = arg_1;
  uint2 v_1 = (0u).xx;
  arg_0.GetDimensions(0u, v_1.x, v_1.y);
  uint v_2 = min(arg_2, (v_1.y - 1u));
  uint2 v_3 = (0u).xx;
  arg_0.GetDimensions(uint(v_2), v_3.x, v_3.y);
  uint v_4 = (v_3.x - 1u);
  int v_5 = int(min(uint(v), v_4));
  float4 res = float4(arg_0.Load(int2(v_5, int(v_2))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_3da3ed()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_3da3ed()));
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = textureLoad_3da3ed();
  VertexOutput v_6 = tint_symbol;
  return v_6;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_7 = vertex_main_inner();
  vertex_main_outputs v_8 = {v_7.prevent_dce, v_7.pos};
  return v_8;
}

