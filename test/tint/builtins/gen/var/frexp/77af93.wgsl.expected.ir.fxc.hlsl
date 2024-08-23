SKIP: FAILED

struct frexp_result_vec4_f32 {
  float4 fract;
  int4 exp;
};

struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void frexp_77af93() {
  float4 arg_0 = (1.0f).xxxx;
  frexp_result_vec4_f32 res = frexp(arg_0);
}

void fragment_main() {
  frexp_77af93();
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_77af93();
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  frexp_77af93();
  VertexOutput v = tint_symbol;
  return v;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_1 = vertex_main_inner();
  vertex_main_outputs v_2 = {v_1.pos};
  return v_2;
}

FXC validation failure:
<scrubbed_path>(17,31-42): error X3013: 'frexp': no matching 1 parameter intrinsic function
<scrubbed_path>(17,31-42): error X3013: Possible intrinsic functions are:
<scrubbed_path>(17,31-42): error X3013:     frexp(float|half, out float|half exp)


tint executable returned error: exit status 1
