Texture2D arg_0 : register(t0, space1);

void textureDimensions_12c9bb() {
  int3 tint_tmp;
  arg_0.GetDimensions(1, tint_tmp.x, tint_tmp.y, tint_tmp.z);
  int2 res = tint_tmp.xy;
}

void vertex_main() {
  textureDimensions_12c9bb();
  return;
}

void fragment_main() {
  textureDimensions_12c9bb();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureDimensions_12c9bb();
  return;
}

