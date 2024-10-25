struct VertexOutput {
  float4 pos;
  uint prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
uint pack2x16snorm_6c169b() {
  float2 arg_0 = (1.0f).xx;
  int2 v = (int2(round((clamp(arg_0, (-1.0f).xx, (1.0f).xx) * 32767.0f))) & (int(65535)).xx);
  uint res = asuint((v.x | (v.y << 16u)));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, pack2x16snorm_6c169b());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, pack2x16snorm_6c169b());
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = pack2x16snorm_6c169b();
  VertexOutput v_1 = tint_symbol;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

