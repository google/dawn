SKIP: FAILED

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


void modf_5ea256() {
  float3 arg_0 = (-1.5f).xxx;
  modf_result_vec3_f32 res = modf(arg_0);
}

void fragment_main() {
  modf_5ea256();
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_5ea256();
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  modf_5ea256();
  VertexOutput v = tint_symbol;
  return v;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_1 = vertex_main_inner();
  vertex_main_outputs v_2 = {v_1.pos};
  return v_2;
}

FXC validation failure:
c:\src\dawn\Shader@0x000001EF90CB6690(17,30-40): error X3013: 'modf': no matching 1 parameter intrinsic function
c:\src\dawn\Shader@0x000001EF90CB6690(17,30-40): error X3013: Possible intrinsic functions are:
c:\src\dawn\Shader@0x000001EF90CB6690(17,30-40): error X3013:     modf(float|half|min10float|min16float, out float|half|min10float|min16float ip)

