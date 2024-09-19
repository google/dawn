struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void ldexp_376938() {
  int4 arg_1 = (int(1)).xxxx;
  float4 res = ldexp((1.0f).xxxx, arg_1);
}

void fragment_main() {
  ldexp_376938();
}

[numthreads(1, 1, 1)]
void compute_main() {
  ldexp_376938();
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  ldexp_376938();
  VertexOutput v = tint_symbol;
  return v;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_1 = vertex_main_inner();
  vertex_main_outputs v_2 = {v_1.pos};
  return v_2;
}

