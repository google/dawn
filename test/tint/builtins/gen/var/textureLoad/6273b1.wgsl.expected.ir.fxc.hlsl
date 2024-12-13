//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DMS<float4> arg_0 : register(t0, space1);
float textureLoad_6273b1() {
  int2 arg_1 = (int(1)).xx;
  int arg_2 = int(1);
  int v = arg_2;
  uint3 v_1 = (0u).xxx;
  arg_0.GetDimensions(v_1.x, v_1.y, v_1.z);
  uint2 v_2 = (v_1.xy - (1u).xx);
  int2 v_3 = int2(min(uint2(arg_1), v_2));
  float res = arg_0.Load(v_3, int(v)).x;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureLoad_6273b1()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DMS<float4> arg_0 : register(t0, space1);
float textureLoad_6273b1() {
  int2 arg_1 = (int(1)).xx;
  int arg_2 = int(1);
  int v = arg_2;
  uint3 v_1 = (0u).xxx;
  arg_0.GetDimensions(v_1.x, v_1.y, v_1.z);
  uint2 v_2 = (v_1.xy - (1u).xx);
  int2 v_3 = int2(min(uint2(arg_1), v_2));
  float res = arg_0.Load(v_3, int(v)).x;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(textureLoad_6273b1()));
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
float textureLoad_6273b1() {
  int2 arg_1 = (int(1)).xx;
  int arg_2 = int(1);
  int v = arg_2;
  uint3 v_1 = (0u).xxx;
  arg_0.GetDimensions(v_1.x, v_1.y, v_1.z);
  uint2 v_2 = (v_1.xy - (1u).xx);
  int2 v_3 = int2(min(uint2(arg_1), v_2));
  float res = arg_0.Load(v_3, int(v)).x;
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_4 = (VertexOutput)0;
  v_4.pos = (0.0f).xxxx;
  v_4.prevent_dce = textureLoad_6273b1();
  VertexOutput v_5 = v_4;
  return v_5;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_6 = vertex_main_inner();
  vertex_main_outputs v_7 = {v_6.prevent_dce, v_6.pos};
  return v_7;
}

