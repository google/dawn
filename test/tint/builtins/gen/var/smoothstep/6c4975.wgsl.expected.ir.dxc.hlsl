//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float smoothstep_6c4975() {
  float arg_0 = 2.0f;
  float arg_1 = 4.0f;
  float arg_2 = 3.0f;
  float v = arg_0;
  float v_1 = clamp(((arg_2 - v) / (arg_1 - v)), 0.0f, 1.0f);
  float res = (v_1 * (v_1 * (3.0f - (2.0f * v_1))));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(smoothstep_6c4975()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float smoothstep_6c4975() {
  float arg_0 = 2.0f;
  float arg_1 = 4.0f;
  float arg_2 = 3.0f;
  float v = arg_0;
  float v_1 = clamp(((arg_2 - v) / (arg_1 - v)), 0.0f, 1.0f);
  float res = (v_1 * (v_1 * (3.0f - (2.0f * v_1))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(smoothstep_6c4975()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  float prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


float smoothstep_6c4975() {
  float arg_0 = 2.0f;
  float arg_1 = 4.0f;
  float arg_2 = 3.0f;
  float v = arg_0;
  float v_1 = clamp(((arg_2 - v) / (arg_1 - v)), 0.0f, 1.0f);
  float res = (v_1 * (v_1 * (3.0f - (2.0f * v_1))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = smoothstep_6c4975();
  VertexOutput v_2 = tint_symbol;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_3.pos};
  return v_4;
}

