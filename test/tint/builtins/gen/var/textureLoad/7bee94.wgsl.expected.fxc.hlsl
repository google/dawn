//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DMS<int4> arg_0 : register(t0, space1);
int4 textureLoad_7bee94() {
  uint2 arg_1 = (1u).xx;
  int arg_2 = int(1);
  uint2 v = arg_1;
  uint3 v_1 = (0u).xxx;
  arg_0.GetDimensions(v_1.x, v_1.y, v_1.z);
  uint v_2 = (v_1.z - 1u);
  uint v_3 = min(uint(arg_2), v_2);
  int2 v_4 = int2(v);
  int4 res = arg_0.Load(v_4, int(v_3));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_7bee94()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DMS<int4> arg_0 : register(t0, space1);
int4 textureLoad_7bee94() {
  uint2 arg_1 = (1u).xx;
  int arg_2 = int(1);
  uint2 v = arg_1;
  uint3 v_1 = (0u).xxx;
  arg_0.GetDimensions(v_1.x, v_1.y, v_1.z);
  uint v_2 = (v_1.z - 1u);
  uint v_3 = min(uint(arg_2), v_2);
  int2 v_4 = int2(v);
  int4 res = arg_0.Load(v_4, int(v_3));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_7bee94()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  int4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


Texture2DMS<int4> arg_0 : register(t0, space1);
int4 textureLoad_7bee94() {
  uint2 arg_1 = (1u).xx;
  int arg_2 = int(1);
  uint2 v = arg_1;
  uint3 v_1 = (0u).xxx;
  arg_0.GetDimensions(v_1.x, v_1.y, v_1.z);
  uint v_2 = (v_1.z - 1u);
  uint v_3 = min(uint(arg_2), v_2);
  int2 v_4 = int2(v);
  int4 res = arg_0.Load(v_4, int(v_3));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_5 = (VertexOutput)0;
  v_5.pos = (0.0f).xxxx;
  v_5.prevent_dce = textureLoad_7bee94();
  VertexOutput v_6 = v_5;
  return v_6;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_7 = vertex_main_inner();
  vertex_main_outputs v_8 = {v_7.prevent_dce, v_7.pos};
  return v_8;
}

