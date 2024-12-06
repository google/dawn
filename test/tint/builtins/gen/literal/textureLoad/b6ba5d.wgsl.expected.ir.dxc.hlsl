//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray arg_0 : register(t0, space1);
float textureLoad_b6ba5d() {
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  uint v_1 = min(uint(int(1)), (v.z - 1u));
  uint4 v_2 = (0u).xxxx;
  arg_0.GetDimensions(0u, v_2.x, v_2.y, v_2.z, v_2.w);
  uint v_3 = min(uint(int(1)), (v_2.w - 1u));
  uint4 v_4 = (0u).xxxx;
  arg_0.GetDimensions(uint(v_3), v_4.x, v_4.y, v_4.z, v_4.w);
  int2 v_5 = int2(min((1u).xx, (v_4.xy - (1u).xx)));
  int v_6 = int(v_1);
  float res = arg_0.Load(int4(v_5, v_6, int(v_3))).x;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureLoad_b6ba5d()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray arg_0 : register(t0, space1);
float textureLoad_b6ba5d() {
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  uint v_1 = min(uint(int(1)), (v.z - 1u));
  uint4 v_2 = (0u).xxxx;
  arg_0.GetDimensions(0u, v_2.x, v_2.y, v_2.z, v_2.w);
  uint v_3 = min(uint(int(1)), (v_2.w - 1u));
  uint4 v_4 = (0u).xxxx;
  arg_0.GetDimensions(uint(v_3), v_4.x, v_4.y, v_4.z, v_4.w);
  int2 v_5 = int2(min((1u).xx, (v_4.xy - (1u).xx)));
  int v_6 = int(v_1);
  float res = arg_0.Load(int4(v_5, v_6, int(v_3))).x;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(textureLoad_b6ba5d()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  float prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


Texture2DArray arg_0 : register(t0, space1);
float textureLoad_b6ba5d() {
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  uint v_1 = min(uint(int(1)), (v.z - 1u));
  uint4 v_2 = (0u).xxxx;
  arg_0.GetDimensions(0u, v_2.x, v_2.y, v_2.z, v_2.w);
  uint v_3 = min(uint(int(1)), (v_2.w - 1u));
  uint4 v_4 = (0u).xxxx;
  arg_0.GetDimensions(uint(v_3), v_4.x, v_4.y, v_4.z, v_4.w);
  int2 v_5 = int2(min((1u).xx, (v_4.xy - (1u).xx)));
  int v_6 = int(v_1);
  float res = arg_0.Load(int4(v_5, v_6, int(v_3))).x;
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = textureLoad_b6ba5d();
  VertexOutput v_7 = tint_symbol;
  return v_7;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_8 = vertex_main_inner();
  vertex_main_outputs v_9 = {v_8.prevent_dce, v_8.pos};
  return v_9;
}

