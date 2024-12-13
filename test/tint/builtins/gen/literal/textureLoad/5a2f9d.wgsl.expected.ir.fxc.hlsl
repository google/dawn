//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture1D<int4> arg_0 : register(t0, space1);
int4 textureLoad_5a2f9d() {
  uint2 v = (0u).xx;
  arg_0.GetDimensions(0u, v.x, v.y);
  uint v_1 = min(uint(int(1)), (v.y - 1u));
  uint2 v_2 = (0u).xx;
  arg_0.GetDimensions(uint(v_1), v_2.x, v_2.y);
  uint v_3 = (v_2.x - 1u);
  int v_4 = int(min(uint(int(1)), v_3));
  int4 res = int4(arg_0.Load(int2(v_4, int(v_1))));
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
  uint2 v = (0u).xx;
  arg_0.GetDimensions(0u, v.x, v.y);
  uint v_1 = min(uint(int(1)), (v.y - 1u));
  uint2 v_2 = (0u).xx;
  arg_0.GetDimensions(uint(v_1), v_2.x, v_2.y);
  uint v_3 = (v_2.x - 1u);
  int v_4 = int(min(uint(int(1)), v_3));
  int4 res = int4(arg_0.Load(int2(v_4, int(v_1))));
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
  uint2 v = (0u).xx;
  arg_0.GetDimensions(0u, v.x, v.y);
  uint v_1 = min(uint(int(1)), (v.y - 1u));
  uint2 v_2 = (0u).xx;
  arg_0.GetDimensions(uint(v_1), v_2.x, v_2.y);
  uint v_3 = (v_2.x - 1u);
  int v_4 = int(min(uint(int(1)), v_3));
  int4 res = int4(arg_0.Load(int2(v_4, int(v_1))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_5 = (VertexOutput)0;
  v_5.pos = (0.0f).xxxx;
  v_5.prevent_dce = textureLoad_5a2f9d();
  VertexOutput v_6 = v_5;
  return v_6;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_7 = vertex_main_inner();
  vertex_main_outputs v_8 = {v_7.prevent_dce, v_7.pos};
  return v_8;
}

