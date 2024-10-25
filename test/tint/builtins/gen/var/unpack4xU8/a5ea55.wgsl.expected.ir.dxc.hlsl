struct VertexOutput {
  float4 pos;
  uint4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
uint4 unpack4xU8_a5ea55() {
  uint arg_0 = 1u;
  uint4 res = unpack_u8u32(uint8_t4_packed(arg_0));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, unpack4xU8_a5ea55());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, unpack4xU8_a5ea55());
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = unpack4xU8_a5ea55();
  VertexOutput v = tint_symbol;
  return v;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_1 = vertex_main_inner();
  vertex_main_outputs v_2 = {v_1.prevent_dce, v_1.pos};
  return v_2;
}

