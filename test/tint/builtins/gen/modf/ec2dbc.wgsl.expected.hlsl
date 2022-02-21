struct modf_result_vec4 {
  float4 fract;
  float4 whole;
};
modf_result_vec4 tint_modf(float4 param_0) {
  float4 whole;
  float4 fract = modf(param_0, whole);
  modf_result_vec4 result = {fract, whole};
  return result;
}

void modf_ec2dbc() {
  modf_result_vec4 res = tint_modf(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  modf_ec2dbc();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  modf_ec2dbc();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_ec2dbc();
  return;
}
