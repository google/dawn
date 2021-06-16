TextureCubeArray<float4> arg_0 : register(t0, space1);

void textureNumLayers_5d59cd() {
  int3 tint_tmp;
  arg_0.GetDimensions(tint_tmp.x, tint_tmp.y, tint_tmp.z);
  int res = tint_tmp.z;
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  textureNumLayers_5d59cd();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  textureNumLayers_5d59cd();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureNumLayers_5d59cd();
  return;
}
