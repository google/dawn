Texture3D<int4> arg_0 : register(t0, space1);

void textureDimensions_8799ee() {
  int3 tint_tmp;
  arg_0.GetDimensions(tint_tmp.x, tint_tmp.y, tint_tmp.z);
  int3 res = tint_tmp;
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  textureDimensions_8799ee();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  textureDimensions_8799ee();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureDimensions_8799ee();
  return;
}
