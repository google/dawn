SKIP: FAILED

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
  modf_result_f16 res = modf(arg_0);
}

void fragment_main() {
  modf_8dbbbf();
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_8dbbbf();
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  modf_8dbbbf();
  VertexOutput v = tint_symbol;
  return v;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_1 = vertex_main_inner();
  vertex_main_outputs v_2 = {v_1.pos};
  return v_2;
}

DXC validation failure:
hlsl.hlsl:17:25: error: use of undeclared identifier 'modf'
  modf_result_f16 res = modf(arg_0);
                        ^


tint executable returned error: exit status 1
