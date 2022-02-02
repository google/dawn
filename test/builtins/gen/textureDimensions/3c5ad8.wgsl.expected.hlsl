RWTexture2D<int4> arg_0 : register(u0, space1);

void textureDimensions_3c5ad8() {
  int2 tint_tmp;
  arg_0.GetDimensions(tint_tmp.x, tint_tmp.y);
  int2 res = tint_tmp;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureDimensions_3c5ad8();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureDimensions_3c5ad8();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureDimensions_3c5ad8();
  return;
}
