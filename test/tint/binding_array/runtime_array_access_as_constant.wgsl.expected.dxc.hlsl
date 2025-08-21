
Texture2D<float4> sampled_textures[] : register(t0);
void fs() {
  float4 texture_load = sampled_textures[int(0)].Load(int3((int(0)).xx, int(0)));
}

