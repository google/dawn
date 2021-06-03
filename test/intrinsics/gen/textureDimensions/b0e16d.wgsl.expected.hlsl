Texture2D<int4> arg_0 : register(t0, space1);

void textureDimensions_b0e16d() {
  int3 tint_tmp;
  arg_0.GetDimensions(1, tint_tmp.x, tint_tmp.y, tint_tmp.z);
  int2 res = tint_tmp.xy;
}

void vertex_main() {
  textureDimensions_b0e16d();
  return;
}

void fragment_main() {
  textureDimensions_b0e16d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureDimensions_b0e16d();
  return;
}

