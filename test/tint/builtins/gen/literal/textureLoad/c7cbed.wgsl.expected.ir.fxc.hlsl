//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture1D<float4> arg_0 : register(t0, space1);
float4 textureLoad_c7cbed() {
  uint v = 0u;
  arg_0.GetDimensions(v);
  uint v_1 = (v - 1u);
  float4 res = float4(arg_0.Load(int2(int(min(uint(int(1)), v_1)), int(0))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_c7cbed()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture1D<float4> arg_0 : register(t0, space1);
float4 textureLoad_c7cbed() {
  uint v = 0u;
  arg_0.GetDimensions(v);
  uint v_1 = (v - 1u);
  float4 res = float4(arg_0.Load(int2(int(min(uint(int(1)), v_1)), int(0))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_c7cbed()));
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


Texture1D<float4> arg_0 : register(t0, space1);
float4 textureLoad_c7cbed() {
  uint v = 0u;
  arg_0.GetDimensions(v);
  uint v_1 = (v - 1u);
  float4 res = float4(arg_0.Load(int2(int(min(uint(int(1)), v_1)), int(0))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = textureLoad_c7cbed();
  VertexOutput v_2 = tint_symbol;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_3.pos};
  return v_4;
}

