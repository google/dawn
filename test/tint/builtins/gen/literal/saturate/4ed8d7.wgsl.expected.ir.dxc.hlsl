struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void saturate_4ed8d7() {
  float4 res = (1.0f).xxxx;
}

void fragment_main() {
  saturate_4ed8d7();
}

[numthreads(1, 1, 1)]
void compute_main() {
  saturate_4ed8d7();
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  saturate_4ed8d7();
  VertexOutput v = tint_symbol;
  return v;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_1 = vertex_main_inner();
  vertex_main_outputs v_2 = {v_1.pos};
  return v_2;
}

