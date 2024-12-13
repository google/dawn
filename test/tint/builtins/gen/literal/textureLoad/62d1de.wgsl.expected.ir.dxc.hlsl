//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture1D<int4> arg_0 : register(t0, space1);
int4 textureLoad_62d1de() {
  uint2 v = (0u).xx;
  arg_0.GetDimensions(0u, v.x, v.y);
  uint2 v_1 = (0u).xx;
  arg_0.GetDimensions(uint(min(1u, (v.y - 1u))), v_1.x, v_1.y);
  uint v_2 = (v_1.x - 1u);
  int v_3 = int(min(uint(int(1)), v_2));
  int4 res = int4(arg_0.Load(int2(v_3, int(min(1u, (v.y - 1u))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_62d1de()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture1D<int4> arg_0 : register(t0, space1);
int4 textureLoad_62d1de() {
  uint2 v = (0u).xx;
  arg_0.GetDimensions(0u, v.x, v.y);
  uint2 v_1 = (0u).xx;
  arg_0.GetDimensions(uint(min(1u, (v.y - 1u))), v_1.x, v_1.y);
  uint v_2 = (v_1.x - 1u);
  int v_3 = int(min(uint(int(1)), v_2));
  int4 res = int4(arg_0.Load(int2(v_3, int(min(1u, (v.y - 1u))))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_62d1de()));
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
int4 textureLoad_62d1de() {
  uint2 v = (0u).xx;
  arg_0.GetDimensions(0u, v.x, v.y);
  uint2 v_1 = (0u).xx;
  arg_0.GetDimensions(uint(min(1u, (v.y - 1u))), v_1.x, v_1.y);
  uint v_2 = (v_1.x - 1u);
  int v_3 = int(min(uint(int(1)), v_2));
  int4 res = int4(arg_0.Load(int2(v_3, int(min(1u, (v.y - 1u))))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_4 = (VertexOutput)0;
  v_4.pos = (0.0f).xxxx;
  v_4.prevent_dce = textureLoad_62d1de();
  VertexOutput v_5 = v_4;
  return v_5;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_6 = vertex_main_inner();
  vertex_main_outputs v_7 = {v_6.prevent_dce, v_6.pos};
  return v_7;
}

