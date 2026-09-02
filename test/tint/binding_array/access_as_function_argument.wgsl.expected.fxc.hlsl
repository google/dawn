
Texture2D<float4> sampled_textures[4] : register(t0);
void do_texture_load(Texture2D<float4> t) {
  float4 texture_load = t.Load((int(0)).xxx);
}

void fs() {
  do_texture_load(sampled_textures[int(0)]);
}

