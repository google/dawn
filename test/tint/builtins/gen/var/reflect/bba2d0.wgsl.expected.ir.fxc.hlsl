struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void reflect_bba2d0() {
  float2 res = (-3.0f).xx;
}

void fragment_main() {
  reflect_bba2d0();
}

[numthreads(1, 1, 1)]
void compute_main() {
  reflect_bba2d0();
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  reflect_bba2d0();
  VertexOutput v = tint_symbol;
  return v;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_1 = vertex_main_inner();
  vertex_main_outputs v_2 = {v_1.pos};
  return v_2;
}

