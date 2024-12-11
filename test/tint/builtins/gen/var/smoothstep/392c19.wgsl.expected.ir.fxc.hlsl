//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 smoothstep_392c19() {
  float2 arg_0 = (2.0f).xx;
  float2 arg_1 = (4.0f).xx;
  float2 arg_2 = (3.0f).xx;
  float2 v = arg_0;
  float2 v_1 = clamp(((arg_2 - v) / (arg_1 - v)), (0.0f).xx, (1.0f).xx);
  float2 res = (v_1 * (v_1 * ((3.0f).xx - ((2.0f).xx * v_1))));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(smoothstep_392c19()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 smoothstep_392c19() {
  float2 arg_0 = (2.0f).xx;
  float2 arg_1 = (4.0f).xx;
  float2 arg_2 = (3.0f).xx;
  float2 v = arg_0;
  float2 v_1 = clamp(((arg_2 - v) / (arg_1 - v)), (0.0f).xx, (1.0f).xx);
  float2 res = (v_1 * (v_1 * ((3.0f).xx - ((2.0f).xx * v_1))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(smoothstep_392c19()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  float2 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float2 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


float2 smoothstep_392c19() {
  float2 arg_0 = (2.0f).xx;
  float2 arg_1 = (4.0f).xx;
  float2 arg_2 = (3.0f).xx;
  float2 v = arg_0;
  float2 v_1 = clamp(((arg_2 - v) / (arg_1 - v)), (0.0f).xx, (1.0f).xx);
  float2 res = (v_1 * (v_1 * ((3.0f).xx - ((2.0f).xx * v_1))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = smoothstep_392c19();
  VertexOutput v_2 = tint_symbol;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_3.pos};
  return v_4;
}

