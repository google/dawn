struct modf_result_vec3_f32 {
  float3 fract;
  float3 whole;
};

struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void modf_68d8ee() {
  modf_result_vec3_f32 res = {(-0.5f).xxx, (-1.0f).xxx};
}

void fragment_main() {
  modf_68d8ee();
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_68d8ee();
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  modf_68d8ee();
  VertexOutput v = tint_symbol;
  return v;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_1 = vertex_main_inner();
  vertex_main_outputs v_2 = {v_1.pos};
  return v_2;
}

