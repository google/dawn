
RWTexture3D<uint4> arg_0 : register(u0, space1);
void textureStore_473ead() {
  arg_0[(1u).xxx] = (1u).xxxx;
}

void fragment_main() {
  textureStore_473ead();
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_473ead();
}

