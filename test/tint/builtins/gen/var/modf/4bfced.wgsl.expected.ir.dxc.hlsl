SKIP: FAILED

struct modf_result_vec4_f32 {
  float4 fract;
  float4 whole;
};

struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void modf_4bfced() {
  float4 arg_0 = (-1.5f).xxxx;
  modf_result_vec4_f32 res = modf(arg_0);
}

void fragment_main() {
  modf_4bfced();
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_4bfced();
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  modf_4bfced();
  VertexOutput v = tint_symbol;
  return v;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_1 = vertex_main_inner();
  vertex_main_outputs v_2 = {v_1.pos};
  return v_2;
}

DXC validation failure:
hlsl.hlsl:17:30: error: use of undeclared identifier 'modf'
  modf_result_vec4_f32 res = modf(arg_0);
                             ^


tint executable returned error: exit status 1
