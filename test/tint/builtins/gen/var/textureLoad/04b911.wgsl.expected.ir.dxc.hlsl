struct VertexOutput {
  float4 pos;
  float prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray arg_0 : register(t0, space1);
float textureLoad_04b911() {
  uint2 arg_1 = (1u).xx;
  int arg_2 = int(1);
  uint arg_3 = 1u;
  uint2 v = arg_1;
  uint v_1 = arg_3;
  uint3 v_2 = (0u).xxx;
  arg_0.GetDimensions(v_2.x, v_2.y, v_2.z);
  uint v_3 = min(uint(arg_2), (v_2.z - 1u));
  uint4 v_4 = (0u).xxxx;
  arg_0.GetDimensions(0u, v_4.x, v_4.y, v_4.z, v_4.w);
  uint4 v_5 = (0u).xxxx;
  arg_0.GetDimensions(uint(min(v_1, (v_4.w - 1u))), v_5.x, v_5.y, v_5.z, v_5.w);
  int2 v_6 = int2(min(v, (v_5.xy - (1u).xx)));
  int v_7 = int(v_3);
  float res = arg_0.Load(int4(v_6, v_7, int(min(v_1, (v_4.w - 1u))))).x;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureLoad_04b911()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(textureLoad_04b911()));
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = textureLoad_04b911();
  VertexOutput v_8 = tint_symbol;
  return v_8;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_9 = vertex_main_inner();
  vertex_main_outputs v_10 = {v_9.prevent_dce, v_9.pos};
  return v_10;
}

