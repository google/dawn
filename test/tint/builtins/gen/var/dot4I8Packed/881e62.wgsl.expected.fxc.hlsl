//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int dot4I8Packed_881e62() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v = arg_1;
  int4 v_1 = (asint((uint4((arg_0).xxxx) << uint4(24u, 16u, 8u, 0u))) >> (24u).xxxx);
  int res = dot(v_1, (asint((uint4((v).xxxx) << uint4(24u, 16u, 8u, 0u))) >> (24u).xxxx));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(dot4I8Packed_881e62()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int dot4I8Packed_881e62() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v = arg_1;
  int4 v_1 = (asint((uint4((arg_0).xxxx) << uint4(24u, 16u, 8u, 0u))) >> (24u).xxxx);
  int res = dot(v_1, (asint((uint4((v).xxxx) << uint4(24u, 16u, 8u, 0u))) >> (24u).xxxx));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(dot4I8Packed_881e62()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  int prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


int dot4I8Packed_881e62() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v = arg_1;
  int4 v_1 = (asint((uint4((arg_0).xxxx) << uint4(24u, 16u, 8u, 0u))) >> (24u).xxxx);
  int res = dot(v_1, (asint((uint4((v).xxxx) << uint4(24u, 16u, 8u, 0u))) >> (24u).xxxx));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_2 = (VertexOutput)0;
  v_2.pos = (0.0f).xxxx;
  v_2.prevent_dce = dot4I8Packed_881e62();
  VertexOutput v_3 = v_2;
  return v_3;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_4 = vertex_main_inner();
  vertex_main_outputs v_5 = {v_4.prevent_dce, v_4.pos};
  return v_5;
}

