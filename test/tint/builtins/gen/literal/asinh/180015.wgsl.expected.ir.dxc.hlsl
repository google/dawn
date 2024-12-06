//
// fragment_main
//

void asinh_180015() {
  float res = 0.88137358427047729492f;
}

void fragment_main() {
  asinh_180015();
}

//
// compute_main
//

void asinh_180015() {
  float res = 0.88137358427047729492f;
}

[numthreads(1, 1, 1)]
void compute_main() {
  asinh_180015();
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void asinh_180015() {
  float res = 0.88137358427047729492f;
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  asinh_180015();
  VertexOutput v = tint_symbol;
  return v;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_1 = vertex_main_inner();
  vertex_main_outputs v_2 = {v_1.pos};
  return v_2;
}

