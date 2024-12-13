//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray arg_0 : register(t0, space1);
float textureLoad_7b63e0() {
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  uint4 v_1 = (0u).xxxx;
  arg_0.GetDimensions(0u, v_1.x, v_1.y, v_1.z, v_1.w);
  uint4 v_2 = (0u).xxxx;
  arg_0.GetDimensions(uint(min(1u, (v_1.w - 1u))), v_2.x, v_2.y, v_2.z, v_2.w);
  int2 v_3 = int2(min((1u).xx, (v_2.xy - (1u).xx)));
  int v_4 = int(min(1u, (v.z - 1u)));
  float res = arg_0.Load(int4(v_3, v_4, int(min(1u, (v_1.w - 1u))))).x;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureLoad_7b63e0()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray arg_0 : register(t0, space1);
float textureLoad_7b63e0() {
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  uint4 v_1 = (0u).xxxx;
  arg_0.GetDimensions(0u, v_1.x, v_1.y, v_1.z, v_1.w);
  uint4 v_2 = (0u).xxxx;
  arg_0.GetDimensions(uint(min(1u, (v_1.w - 1u))), v_2.x, v_2.y, v_2.z, v_2.w);
  int2 v_3 = int2(min((1u).xx, (v_2.xy - (1u).xx)));
  int v_4 = int(min(1u, (v.z - 1u)));
  float res = arg_0.Load(int4(v_3, v_4, int(min(1u, (v_1.w - 1u))))).x;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(textureLoad_7b63e0()));
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
float textureLoad_7b63e0() {
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  uint4 v_1 = (0u).xxxx;
  arg_0.GetDimensions(0u, v_1.x, v_1.y, v_1.z, v_1.w);
  uint4 v_2 = (0u).xxxx;
  arg_0.GetDimensions(uint(min(1u, (v_1.w - 1u))), v_2.x, v_2.y, v_2.z, v_2.w);
  int2 v_3 = int2(min((1u).xx, (v_2.xy - (1u).xx)));
  int v_4 = int(min(1u, (v.z - 1u)));
  float res = arg_0.Load(int4(v_3, v_4, int(min(1u, (v_1.w - 1u))))).x;
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_5 = (VertexOutput)0;
  v_5.pos = (0.0f).xxxx;
  v_5.prevent_dce = textureLoad_7b63e0();
  VertexOutput v_6 = v_5;
  return v_6;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_7 = vertex_main_inner();
  vertex_main_outputs v_8 = {v_7.prevent_dce, v_7.pos};
  return v_8;
}

