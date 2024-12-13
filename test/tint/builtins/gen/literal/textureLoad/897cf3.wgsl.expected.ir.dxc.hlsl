//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2D<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_897cf3() {
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(0u, v.x, v.y, v.z);
  uint3 v_1 = (0u).xxx;
  arg_0.GetDimensions(uint(min(1u, (v.z - 1u))), v_1.x, v_1.y, v_1.z);
  int2 v_2 = int2(min((1u).xx, (v_1.xy - (1u).xx)));
  uint4 res = uint4(arg_0.Load(int3(v_2, int(min(1u, (v.z - 1u))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, textureLoad_897cf3());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2D<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_897cf3() {
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(0u, v.x, v.y, v.z);
  uint3 v_1 = (0u).xxx;
  arg_0.GetDimensions(uint(min(1u, (v.z - 1u))), v_1.x, v_1.y, v_1.z);
  int2 v_2 = int2(min((1u).xx, (v_1.xy - (1u).xx)));
  uint4 res = uint4(arg_0.Load(int3(v_2, int(min(1u, (v.z - 1u))))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, textureLoad_897cf3());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  uint4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


Texture2D<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_897cf3() {
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(0u, v.x, v.y, v.z);
  uint3 v_1 = (0u).xxx;
  arg_0.GetDimensions(uint(min(1u, (v.z - 1u))), v_1.x, v_1.y, v_1.z);
  int2 v_2 = int2(min((1u).xx, (v_1.xy - (1u).xx)));
  uint4 res = uint4(arg_0.Load(int3(v_2, int(min(1u, (v.z - 1u))))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_3 = (VertexOutput)0;
  v_3.pos = (0.0f).xxxx;
  v_3.prevent_dce = textureLoad_897cf3();
  VertexOutput v_4 = v_3;
  return v_4;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_5 = vertex_main_inner();
  vertex_main_outputs v_6 = {v_5.prevent_dce, v_5.pos};
  return v_6;
}

