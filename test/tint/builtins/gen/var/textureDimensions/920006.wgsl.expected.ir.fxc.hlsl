//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture1D<uint4> arg_0 : register(t0, space1);
uint textureDimensions_920006() {
  int arg_1 = int(1);
  uint2 v = (0u).xx;
  arg_0.GetDimensions(0u, v.x, v.y);
  uint2 v_1 = (0u).xx;
  arg_0.GetDimensions(uint(min(uint(arg_1), (v.y - 1u))), v_1.x, v_1.y);
  uint res = v_1.x;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, textureDimensions_920006());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture1D<uint4> arg_0 : register(t0, space1);
uint textureDimensions_920006() {
  int arg_1 = int(1);
  uint2 v = (0u).xx;
  arg_0.GetDimensions(0u, v.x, v.y);
  uint2 v_1 = (0u).xx;
  arg_0.GetDimensions(uint(min(uint(arg_1), (v.y - 1u))), v_1.x, v_1.y);
  uint res = v_1.x;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, textureDimensions_920006());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  uint prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


Texture1D<uint4> arg_0 : register(t0, space1);
uint textureDimensions_920006() {
  int arg_1 = int(1);
  uint2 v = (0u).xx;
  arg_0.GetDimensions(0u, v.x, v.y);
  uint2 v_1 = (0u).xx;
  arg_0.GetDimensions(uint(min(uint(arg_1), (v.y - 1u))), v_1.x, v_1.y);
  uint res = v_1.x;
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_2 = (VertexOutput)0;
  v_2.pos = (0.0f).xxxx;
  v_2.prevent_dce = textureDimensions_920006();
  VertexOutput v_3 = v_2;
  return v_3;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_4 = vertex_main_inner();
  vertex_main_outputs v_5 = {v_4.prevent_dce, v_4.pos};
  return v_5;
}

