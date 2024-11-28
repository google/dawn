struct VertexOutput {
  float4 pos;
  uint2 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint2 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
Texture2D arg_0 : register(t0, space1);
uint2 textureDimensions_991ea9() {
  uint arg_1 = 1u;
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(0u, v.x, v.y, v.z);
  uint3 v_1 = (0u).xxx;
  arg_0.GetDimensions(uint(min(arg_1, (v.z - 1u))), v_1.x, v_1.y, v_1.z);
  uint2 res = v_1.xy;
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, textureDimensions_991ea9());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, textureDimensions_991ea9());
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = textureDimensions_991ea9();
  VertexOutput v_2 = tint_symbol;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_3.pos};
  return v_4;
}

