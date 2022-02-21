TextureCubeArray arg_0 : register(t0, space1);

void textureDimensions_a01845() {
  int4 tint_tmp;
  arg_0.GetDimensions(0, tint_tmp.x, tint_tmp.y, tint_tmp.z, tint_tmp.w);
  int2 res = tint_tmp.xy;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureDimensions_a01845();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureDimensions_a01845();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureDimensions_a01845();
  return;
}
