//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture1D<int4> arg_0 : register(t0, space1);
int4 textureLoad_5a2f9d() {
  int arg_1 = int(1);
  int arg_2 = int(1);
  int v = arg_1;
  uint2 v_1 = (0u).xx;
  arg_0.GetDimensions(0u, v_1.x, v_1.y);
  uint v_2 = min(uint(arg_2), (v_1.y - 1u));
  uint2 v_3 = (0u).xx;
  arg_0.GetDimensions(uint(v_2), v_3.x, v_3.y);
  uint v_4 = (v_3.x - 1u);
  int v_5 = int(min(uint(v), v_4));
  int4 res = int4(arg_0.Load(int2(v_5, int(v_2))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_5a2f9d()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture1D<int4> arg_0 : register(t0, space1);
int4 textureLoad_5a2f9d() {
  int arg_1 = int(1);
  int arg_2 = int(1);
  int v = arg_1;
  uint2 v_1 = (0u).xx;
  arg_0.GetDimensions(0u, v_1.x, v_1.y);
  uint v_2 = min(uint(arg_2), (v_1.y - 1u));
  uint2 v_3 = (0u).xx;
  arg_0.GetDimensions(uint(v_2), v_3.x, v_3.y);
  uint v_4 = (v_3.x - 1u);
  int v_5 = int(min(uint(v), v_4));
  int4 res = int4(arg_0.Load(int2(v_5, int(v_2))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_5a2f9d()));
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


Texture1D<int4> arg_0 : register(t0, space1);
int4 textureLoad_5a2f9d() {
  int arg_1 = int(1);
  int arg_2 = int(1);
  int v = arg_1;
  uint2 v_1 = (0u).xx;
  arg_0.GetDimensions(0u, v_1.x, v_1.y);
  uint v_2 = min(uint(arg_2), (v_1.y - 1u));
  uint2 v_3 = (0u).xx;
  arg_0.GetDimensions(uint(v_2), v_3.x, v_3.y);
  uint v_4 = (v_3.x - 1u);
  int v_5 = int(min(uint(v), v_4));
  int4 res = int4(arg_0.Load(int2(v_5, int(v_2))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_6 = (VertexOutput)0;
  v_6.pos = (0.0f).xxxx;
  v_6.prevent_dce = textureLoad_5a2f9d();
  VertexOutput v_7 = v_6;
  return v_7;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_8 = vertex_main_inner();
  vertex_main_outputs v_9 = {v_8.prevent_dce, v_8.pos};
  return v_9;
}

