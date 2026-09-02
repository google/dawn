
Texture2D<float4> sampled_textures[4] : register(t0);
void do_texture_load(Texture2D<float4> ts[4]) {
  float4 texture_load = ts[int(0)].Load((int(0)).xxx);
}

void fs() {
  do_texture_load(sampled_textures);
}

