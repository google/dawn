struct VertexOutput {
  float4 pos;
  uint prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
uint pack4x8unorm_95c456() {
  float4 arg_0 = (1.0f).xxxx;
  uint4 v = uint4(round((clamp(arg_0, (0.0f).xxxx, (1.0f).xxxx) * 255.0f)));
  uint res = (v.x | ((v.y << 8u) | ((v.z << 16u) | (v.w << 24u))));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, pack4x8unorm_95c456());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, pack4x8unorm_95c456());
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = pack4x8unorm_95c456();
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

