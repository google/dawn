//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int extractBits_249874() {
  int arg_0 = int(1);
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  int v = arg_0;
  uint v_1 = min(arg_1, 32u);
  uint v_2 = (32u - min(32u, (v_1 + min(arg_2, 32u))));
  uint v_3 = (v_2 + v_1);
  int v_4 = select((v_2 < 32u), asint((asuint(v) << uint(v_2))), int(0));
  int res = select((v_3 < 32u), (v_4 >> uint(v_3)), ((v_4 >> 31u) >> 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(extractBits_249874()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int extractBits_249874() {
  int arg_0 = int(1);
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  int v = arg_0;
  uint v_1 = min(arg_1, 32u);
  uint v_2 = (32u - min(32u, (v_1 + min(arg_2, 32u))));
  uint v_3 = (v_2 + v_1);
  int v_4 = select((v_2 < 32u), asint((asuint(v) << uint(v_2))), int(0));
  int res = select((v_3 < 32u), (v_4 >> uint(v_3)), ((v_4 >> 31u) >> 1u));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(extractBits_249874()));
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


int extractBits_249874() {
  int arg_0 = int(1);
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  int v = arg_0;
  uint v_1 = min(arg_1, 32u);
  uint v_2 = (32u - min(32u, (v_1 + min(arg_2, 32u))));
  uint v_3 = (v_2 + v_1);
  int v_4 = select((v_2 < 32u), asint((asuint(v) << uint(v_2))), int(0));
  int res = select((v_3 < 32u), (v_4 >> uint(v_3)), ((v_4 >> 31u) >> 1u));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_5 = (VertexOutput)0;
  v_5.pos = (0.0f).xxxx;
  v_5.prevent_dce = extractBits_249874();
  VertexOutput v_6 = v_5;
  return v_6;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_7 = vertex_main_inner();
  vertex_main_outputs v_8 = {v_7.prevent_dce, v_7.pos};
  return v_8;
}

