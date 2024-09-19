struct VertexOutput {
  float4 pos;
  int prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
int insertBits_65468b() {
  int arg_0 = int(1);
  int arg_1 = int(1);
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int v = arg_0;
  int v_1 = arg_1;
  uint v_2 = arg_2;
  uint v_3 = (v_2 + arg_3);
  uint v_4 = (((v_2 < 32u)) ? ((1u << v_2)) : (0u));
  uint v_5 = ((v_4 - 1u) ^ ((((v_3 < 32u)) ? ((1u << v_3)) : (0u)) - 1u));
  int v_6 = (((v_2 < 32u)) ? ((v_1 << uint(v_2))) : (int(0)));
  int v_7 = (v_6 & int(v_5));
  int res = (v_7 | (v & int(~(v_5))));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(insertBits_65468b()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(insertBits_65468b()));
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = insertBits_65468b();
  VertexOutput v_8 = tint_symbol;
  return v_8;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_9 = vertex_main_inner();
  VertexOutput v_10 = v_9;
  VertexOutput v_11 = v_9;
  vertex_main_outputs v_12 = {v_11.prevent_dce, v_10.pos};
  return v_12;
}

