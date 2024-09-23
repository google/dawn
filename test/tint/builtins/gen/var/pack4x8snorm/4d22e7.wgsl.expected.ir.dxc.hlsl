struct VertexOutput {
  float4 pos;
  uint prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
uint pack4x8snorm_4d22e7() {
  float4 arg_0 = (1.0f).xxxx;
  int4 v = (int4(round((clamp(arg_0, (-1.0f).xxxx, (1.0f).xxxx) * 127.0f))) & (int(255)).xxxx);
  uint res = asuint((v.x | ((v.y << 8u) | ((v.z << 16u) | (v.w << 24u)))));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, pack4x8snorm_4d22e7());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, pack4x8snorm_4d22e7());
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = pack4x8snorm_4d22e7();
  VertexOutput v_1 = tint_symbol;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  VertexOutput v_3 = v_2;
  VertexOutput v_4 = v_2;
  vertex_main_outputs v_5 = {v_4.prevent_dce, v_3.pos};
  return v_5;
}

