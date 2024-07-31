struct modf_result_vec2_f32 {
  float2 fract;
  float2 whole;
};

struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void modf_2d50da() {
  modf_result_vec2_f32 res = {(-0.5f).xx, (-1.0f).xx};
}

void fragment_main() {
  modf_2d50da();
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_2d50da();
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  modf_2d50da();
  VertexOutput v = tint_symbol;
  return v;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_1 = vertex_main_inner();
  vertex_main_outputs v_2 = {v_1.pos};
  return v_2;
}

