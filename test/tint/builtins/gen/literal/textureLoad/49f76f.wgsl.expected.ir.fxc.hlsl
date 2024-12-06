//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DMS<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_49f76f() {
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  int2 v_1 = int2(min((1u).xx, (v.xy - (1u).xx)));
  uint4 res = uint4(arg_0.Load(v_1, int(1u)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, textureLoad_49f76f());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DMS<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_49f76f() {
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  int2 v_1 = int2(min((1u).xx, (v.xy - (1u).xx)));
  uint4 res = uint4(arg_0.Load(v_1, int(1u)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, textureLoad_49f76f());
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


Texture2DMS<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_49f76f() {
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  int2 v_1 = int2(min((1u).xx, (v.xy - (1u).xx)));
  uint4 res = uint4(arg_0.Load(v_1, int(1u)));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = textureLoad_49f76f();
  VertexOutput v_2 = tint_symbol;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_3.pos};
  return v_4;
}

