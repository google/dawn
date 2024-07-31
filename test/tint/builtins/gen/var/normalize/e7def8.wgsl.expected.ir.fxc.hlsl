struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void normalize_e7def8() {
  float3 res = (0.57735025882720947266f).xxx;
}

void fragment_main() {
  normalize_e7def8();
}

[numthreads(1, 1, 1)]
void compute_main() {
  normalize_e7def8();
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  normalize_e7def8();
  VertexOutput v = tint_symbol;
  return v;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_1 = vertex_main_inner();
  vertex_main_outputs v_2 = {v_1.pos};
  return v_2;
}

