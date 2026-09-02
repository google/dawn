//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<int4> arg_0 : register(t0, space1);
int4 textureLoad_9885b0() {
  int2 v = int2((1u).xx);
  int v_1 = int(1u);
  int v_2 = int(1u);
  int4 res = arg_0.Load((int(1)).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_9885b0()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<int4> arg_0 : register(t0, space1);
int4 textureLoad_9885b0() {
  int2 v = int2((1u).xx);
  int v_1 = int(1u);
  int v_2 = int(1u);
  int4 res = arg_0.Load((int(1)).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_9885b0()));
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


Texture2DArray<int4> arg_0 : register(t0, space1);
int4 textureLoad_9885b0() {
  int2 v = int2((1u).xx);
  int v_1 = int(1u);
  int v_2 = int(1u);
  int4 res = arg_0.Load((int(1)).xxxx);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_3 = (VertexOutput)0;
  v_3.pos = (0.0f).xxxx;
  v_3.prevent_dce = textureLoad_9885b0();
  VertexOutput v_4 = v_3;
  return v_4;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_5 = vertex_main_inner();
  vertex_main_outputs v_6 = {v_5.prevent_dce, v_5.pos};
  return v_6;
}

