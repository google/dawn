//
// fragment_main
//

void length_c2c544() {
  float res = 0.0f;
}

void fragment_main() {
  length_c2c544();
}

//
// compute_main
//

void length_c2c544() {
  float res = 0.0f;
}

[numthreads(1, 1, 1)]
void compute_main() {
  length_c2c544();
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


void length_c2c544() {
  float res = 0.0f;
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  length_c2c544();
  VertexOutput v = tint_symbol;
  return v;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_1 = vertex_main_inner();
  vertex_main_outputs v_2 = {v_1.pos};
  return v_2;
}

