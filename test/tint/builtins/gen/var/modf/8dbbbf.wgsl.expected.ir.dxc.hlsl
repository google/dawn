//
// fragment_main
//
struct modf_result_f16 {
  float16_t fract;
  float16_t whole;
};


void modf_8dbbbf() {
  float16_t arg_0 = float16_t(-1.5h);
  float16_t v = float16_t(0.0h);
  modf_result_f16 res = {modf(arg_0, v), v};
}

void fragment_main() {
  modf_8dbbbf();
}

//
// compute_main
//
struct modf_result_f16 {
  float16_t fract;
  float16_t whole;
};


void modf_8dbbbf() {
  float16_t arg_0 = float16_t(-1.5h);
  float16_t v = float16_t(0.0h);
  modf_result_f16 res = {modf(arg_0, v), v};
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_8dbbbf();
}

//
// vertex_main
//
struct modf_result_f16 {
  float16_t fract;
  float16_t whole;
};

struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void modf_8dbbbf() {
  float16_t arg_0 = float16_t(-1.5h);
  float16_t v = float16_t(0.0h);
  modf_result_f16 res = {modf(arg_0, v), v};
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  modf_8dbbbf();
  VertexOutput v_1 = tint_symbol;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.pos};
  return v_3;
}

