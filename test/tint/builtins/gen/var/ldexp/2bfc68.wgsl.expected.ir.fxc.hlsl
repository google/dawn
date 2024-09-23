struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void ldexp_2bfc68() {
  int2 arg_1 = (int(1)).xx;
  float2 res = ldexp((1.0f).xx, arg_1);
}

void fragment_main() {
  ldexp_2bfc68();
}

[numthreads(1, 1, 1)]
void compute_main() {
  ldexp_2bfc68();
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  ldexp_2bfc68();
  VertexOutput v = tint_symbol;
  return v;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_1 = vertex_main_inner();
  vertex_main_outputs v_2 = {v_1.pos};
  return v_2;
}

