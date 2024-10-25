struct VertexOutput {
  float4 pos;
  float4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
float4 unpack4x8unorm_750c74() {
  uint arg_0 = 1u;
  uint v = arg_0;
  float4 res = (float4(uint4((v & 255u), ((v >> 8u) & 255u), ((v >> 16u) & 255u), (v >> 24u))) / 255.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(unpack4x8unorm_750c74()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(unpack4x8unorm_750c74()));
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = unpack4x8unorm_750c74();
  VertexOutput v_1 = tint_symbol;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

