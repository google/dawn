//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<float4> arg_0 : register(t0, space1);
float4 textureLoad_46a93f() {
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
  float4 res = float4(arg_0.Load(int4(v_6, v_7, int(min(v_1, (v_4.w - 1u))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_46a93f()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<float4> arg_0 : register(t0, space1);
float4 textureLoad_46a93f() {
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
  float4 res = float4(arg_0.Load(int4(v_6, v_7, int(min(v_1, (v_4.w - 1u))))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_46a93f()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  float4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


Texture2DArray<float4> arg_0 : register(t0, space1);
float4 textureLoad_46a93f() {
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
  float4 res = float4(arg_0.Load(int4(v_6, v_7, int(min(v_1, (v_4.w - 1u))))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_8 = (VertexOutput)0;
  v_8.pos = (0.0f).xxxx;
  v_8.prevent_dce = textureLoad_46a93f();
  VertexOutput v_9 = v_8;
  return v_9;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_10 = vertex_main_inner();
  vertex_main_outputs v_11 = {v_10.prevent_dce, v_10.pos};
  return v_11;
}

