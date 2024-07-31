struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void select_4e60da() {
  bool arg_2 = true;
  float2 res = ((arg_2) ? ((1.0f).xx) : ((1.0f).xx));
}

void fragment_main() {
  select_4e60da();
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_4e60da();
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  select_4e60da();
  VertexOutput v = tint_symbol;
  return v;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_1 = vertex_main_inner();
  vertex_main_outputs v_2 = {v_1.pos};
  return v_2;
}

