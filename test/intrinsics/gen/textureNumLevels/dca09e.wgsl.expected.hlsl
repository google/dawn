Texture3D<float4> arg_0 : register(t0, space1);

void textureNumLevels_dca09e() {
  int4 tint_tmp;
  arg_0.GetDimensions(0, tint_tmp.x, tint_tmp.y, tint_tmp.z, tint_tmp.w);
  int res = tint_tmp.w;
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  textureNumLevels_dca09e();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  textureNumLevels_dca09e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureNumLevels_dca09e();
  return;
}
