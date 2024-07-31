struct VertexOutput {
  float4 pos;
  uint prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
uint dot4U8Packed_fbed7b() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v = arg_0;
  uint v_1 = arg_1;
  uint4 v_2 = uint4(0u, 8u, 16u, 24u);
  uint4 v_3 = (uint4((v).xxxx) >> v_2);
  uint4 v_4 = (v_3 & uint4((255u).xxxx));
  uint4 v_5 = uint4(0u, 8u, 16u, 24u);
  uint4 v_6 = (uint4((v_1).xxxx) >> v_5);
  uint res = dot(v_4, (v_6 & uint4((255u).xxxx)));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, dot4U8Packed_fbed7b());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, dot4U8Packed_fbed7b());
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = dot4U8Packed_fbed7b();
  VertexOutput v_7 = tint_symbol;
  return v_7;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_8 = vertex_main_inner();
  VertexOutput v_9 = v_8;
  VertexOutput v_10 = v_8;
  vertex_main_outputs v_11 = {v_10.prevent_dce, v_9.pos};
  return v_11;
}

