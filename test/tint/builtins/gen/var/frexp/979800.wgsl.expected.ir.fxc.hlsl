struct frexp_result_vec3_f32 {
  float3 fract;
  int3 exp;
};

struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void frexp_979800() {
  float3 arg_0 = (1.0f).xxx;
  float3 v = arg_0;
  float3 v_1 = (0.0f).xxx;
  float3 v_2 = frexp(v, v_1);
  float3 v_3 = (float3(sign(v)) * v_2);
  frexp_result_vec3_f32 res = {v_3, int3(v_1)};
}

void fragment_main() {
  frexp_979800();
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_979800();
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  frexp_979800();
  VertexOutput v_4 = tint_symbol;
  return v_4;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_5 = vertex_main_inner();
  vertex_main_outputs v_6 = {v_5.pos};
  return v_6;
}

