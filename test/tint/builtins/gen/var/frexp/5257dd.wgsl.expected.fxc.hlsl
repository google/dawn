SKIP: INVALID

//
// fragment_main
//
struct frexp_result_f16 {
  float16_t fract;
  int exp;
};


void frexp_5257dd() {
  float16_t arg_0 = float16_t(1.0h);
  float16_t v = arg_0;
  float16_t v_1 = float16_t(0.0h);
  float16_t v_2 = frexp(v, v_1);
  float16_t v_3 = (float16_t(sign(v)) * v_2);
  frexp_result_f16 res = {v_3, int(v_1)};
}

void fragment_main() {
  frexp_5257dd();
}

//
// compute_main
//
struct frexp_result_f16 {
  float16_t fract;
  int exp;
};


void frexp_5257dd() {
  float16_t arg_0 = float16_t(1.0h);
  float16_t v = arg_0;
  float16_t v_1 = float16_t(0.0h);
  float16_t v_2 = frexp(v, v_1);
  float16_t v_3 = (float16_t(sign(v)) * v_2);
  frexp_result_f16 res = {v_3, int(v_1)};
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_5257dd();
}

//
// vertex_main
//
struct frexp_result_f16 {
  float16_t fract;
  int exp;
};

struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void frexp_5257dd() {
  float16_t arg_0 = float16_t(1.0h);
  float16_t v = arg_0;
  float16_t v_1 = float16_t(0.0h);
  float16_t v_2 = frexp(v, v_1);
  float16_t v_3 = (float16_t(sign(v)) * v_2);
  frexp_result_f16 res = {v_3, int(v_1)};
}

VertexOutput vertex_main_inner() {
  VertexOutput v_4 = (VertexOutput)0;
  v_4.pos = (0.0f).xxxx;
  frexp_5257dd();
  VertexOutput v_5 = v_4;
  return v_5;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_6 = vertex_main_inner();
  vertex_main_outputs v_7 = {v_6.pos};
  return v_7;
}

