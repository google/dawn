
Texture2D<float4> sampled_textures[] : register(t0);
void do_texture_load(Texture2D<float4> t) {
  float4 texture_load = t.Load(int3((int(0)).xx, int(0)));
}

void fs() {
  do_texture_load(sampled_textures[int(0)]);
}

