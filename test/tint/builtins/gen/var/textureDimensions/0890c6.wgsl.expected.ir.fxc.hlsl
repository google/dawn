//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture3D<float4> arg_0 : register(t0, space1);
uint3 textureDimensions_0890c6() {
  uint arg_1 = 1u;
  uint4 v = (0u).xxxx;
  arg_0.GetDimensions(0u, v.x, v.y, v.z, v.w);
  uint4 v_1 = (0u).xxxx;
  arg_0.GetDimensions(uint(min(arg_1, (v.w - 1u))), v_1.x, v_1.y, v_1.z, v_1.w);
  uint3 res = v_1.xyz;
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, textureDimensions_0890c6());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture3D<float4> arg_0 : register(t0, space1);
uint3 textureDimensions_0890c6() {
  uint arg_1 = 1u;
  uint4 v = (0u).xxxx;
  arg_0.GetDimensions(0u, v.x, v.y, v.z, v.w);
  uint4 v_1 = (0u).xxxx;
  arg_0.GetDimensions(uint(min(arg_1, (v.w - 1u))), v_1.x, v_1.y, v_1.z, v_1.w);
  uint3 res = v_1.xyz;
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, textureDimensions_0890c6());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  uint3 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint3 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


Texture3D<float4> arg_0 : register(t0, space1);
uint3 textureDimensions_0890c6() {
  uint arg_1 = 1u;
  uint4 v = (0u).xxxx;
  arg_0.GetDimensions(0u, v.x, v.y, v.z, v.w);
  uint4 v_1 = (0u).xxxx;
  arg_0.GetDimensions(uint(min(arg_1, (v.w - 1u))), v_1.x, v_1.y, v_1.z, v_1.w);
  uint3 res = v_1.xyz;
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_2 = (VertexOutput)0;
  v_2.pos = (0.0f).xxxx;
  v_2.prevent_dce = textureDimensions_0890c6();
  VertexOutput v_3 = v_2;
  return v_3;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_4 = vertex_main_inner();
  vertex_main_outputs v_5 = {v_4.prevent_dce, v_4.pos};
  return v_5;
}

