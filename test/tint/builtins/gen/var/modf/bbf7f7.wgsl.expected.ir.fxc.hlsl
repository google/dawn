SKIP: FAILED

struct modf_result_f32 {
  float fract;
  float whole;
};

struct VertexOutput {
  float4 pos;
};

struct vertex_main_outputs {
  float4 VertexOutput_pos : SV_Position;
};


void modf_bbf7f7() {
  float arg_0 = -1.5f;
  modf_result_f32 res = modf(arg_0);
}

void fragment_main() {
  modf_bbf7f7();
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_bbf7f7();
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  modf_bbf7f7();
  VertexOutput v = tint_symbol;
  return v;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_1 = vertex_main_inner();
  vertex_main_outputs v_2 = {v_1.pos};
  return v_2;
}

FXC validation failure:
c:\src\dawn\Shader@0x000001D2B124D2B0(17,25-35): error X3013: 'modf': no matching 1 parameter intrinsic function
c:\src\dawn\Shader@0x000001D2B124D2B0(17,25-35): error X3013: Possible intrinsic functions are:
c:\src\dawn\Shader@0x000001D2B124D2B0(17,25-35): error X3013:     modf(float|half|min10float|min16float, out float|half|min10float|min16float ip)

