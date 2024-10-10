struct modf_result_vec4_f32 {
  float4 fract;
  float4 whole;
};

struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void modf_4bfced() {
  float4 arg_0 = (-1.5f).xxxx;
  float4 v = (0.0f).xxxx;
  float4 v_1 = modf(arg_0, v);
  modf_result_vec4_f32 res = {v_1, v};
}

void fragment_main() {
  modf_4bfced();
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_4bfced();
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  modf_4bfced();
  VertexOutput v_2 = tint_symbol;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.pos};
  return v_4;
}

