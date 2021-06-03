RWTexture2D<float4> arg_0 : register(u0, space1);

void textureDimensions_4267ee() {
  int2 tint_tmp;
  arg_0.GetDimensions(tint_tmp.x, tint_tmp.y);
  int2 res = tint_tmp;
}

void vertex_main() {
  textureDimensions_4267ee();
  return;
}

void fragment_main() {
  textureDimensions_4267ee();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureDimensions_4267ee();
  return;
}

