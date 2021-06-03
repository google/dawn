Texture3D<float4> arg_0 : register(t0, space1);

void textureDimensions_26bdfa() {
  int4 tint_tmp;
  arg_0.GetDimensions(1, tint_tmp.x, tint_tmp.y, tint_tmp.z, tint_tmp.w);
  int3 res = tint_tmp.xyz;
}

void vertex_main() {
  textureDimensions_26bdfa();
  return;
}

void fragment_main() {
  textureDimensions_26bdfa();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureDimensions_26bdfa();
  return;
}

