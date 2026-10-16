//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DMS<float4> arg_0 : register(t0, space1);
float textureLoad_4db25c() {
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  uint v_1 = min(arg_2, (v.z - 1u));
  int2 v_2 = int2(arg_1);
  float res = arg_0.Load(v_2, int(v_1)).x;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureLoad_4db25c()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DMS<float4> arg_0 : register(t0, space1);
float textureLoad_4db25c() {
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  uint v_1 = min(arg_2, (v.z - 1u));
  int2 v_2 = int2(arg_1);
  float res = arg_0.Load(v_2, int(v_1)).x;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(textureLoad_4db25c()));
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


Texture2DMS<float4> arg_0 : register(t0, space1);
float textureLoad_4db25c() {
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  uint v_1 = min(arg_2, (v.z - 1u));
  int2 v_2 = int2(arg_1);
  float res = arg_0.Load(v_2, int(v_1)).x;
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_3 = (VertexOutput)0;
  v_3.pos = (0.0f).xxxx;
  v_3.prevent_dce = textureLoad_4db25c();
  VertexOutput v_4 = v_3;
  return v_4;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_5 = vertex_main_inner();
  vertex_main_outputs v_6 = {v_5.prevent_dce, v_5.pos};
  return v_6;
}

