//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture1D<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_216c37() {
  uint2 v = (0u).xx;
  arg_0.GetDimensions(0u, v.x, v.y);
  uint v_1 = min(uint(int(1)), (v.y - 1u));
  uint2 v_2 = (0u).xx;
  arg_0.GetDimensions(uint(v_1), v_2.x, v_2.y);
  int v_3 = int(min(1u, (v_2.x - 1u)));
  uint4 res = uint4(arg_0.Load(int2(v_3, int(v_1))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, textureLoad_216c37());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture1D<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_216c37() {
  uint2 v = (0u).xx;
  arg_0.GetDimensions(0u, v.x, v.y);
  uint v_1 = min(uint(int(1)), (v.y - 1u));
  uint2 v_2 = (0u).xx;
  arg_0.GetDimensions(uint(v_1), v_2.x, v_2.y);
  int v_3 = int(min(1u, (v_2.x - 1u)));
  uint4 res = uint4(arg_0.Load(int2(v_3, int(v_1))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, textureLoad_216c37());
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


Texture1D<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_216c37() {
  uint2 v = (0u).xx;
  arg_0.GetDimensions(0u, v.x, v.y);
  uint v_1 = min(uint(int(1)), (v.y - 1u));
  uint2 v_2 = (0u).xx;
  arg_0.GetDimensions(uint(v_1), v_2.x, v_2.y);
  int v_3 = int(min(1u, (v_2.x - 1u)));
  uint4 res = uint4(arg_0.Load(int2(v_3, int(v_1))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_4 = (VertexOutput)0;
  v_4.pos = (0.0f).xxxx;
  v_4.prevent_dce = textureLoad_216c37();
  VertexOutput v_5 = v_4;
  return v_5;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_6 = vertex_main_inner();
  vertex_main_outputs v_7 = {v_6.prevent_dce, v_6.pos};
  return v_7;
}

