Texture2DMS<float4> arg_0 : register(t0, space1);

void textureDimensions_ef5b89() {
  int3 tint_tmp;
  arg_0.GetDimensions(tint_tmp.x, tint_tmp.y, tint_tmp.z);
  int2 res = tint_tmp.xy;
}

void vertex_main() {
  textureDimensions_ef5b89();
  return;
}

void fragment_main() {
  textureDimensions_ef5b89();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureDimensions_ef5b89();
  return;
}

