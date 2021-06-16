RWTexture2DArray<float4> arg_0 : register(u0, space1);

void textureDimensions_e9e96c() {
  int3 tint_tmp;
  arg_0.GetDimensions(tint_tmp.x, tint_tmp.y, tint_tmp.z);
  int2 res = tint_tmp.xy;
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  textureDimensions_e9e96c();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  textureDimensions_e9e96c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureDimensions_e9e96c();
  return;
}
