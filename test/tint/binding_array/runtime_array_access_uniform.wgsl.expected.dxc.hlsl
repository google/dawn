
cbuffer cbuffer_index : register(b0, space1) {
  uint4 index[1];
};
Texture2D<float4> sampled_textures[] : register(t0);
void fs() {
  uint v = index[0u].x;
  float4 texture_load = sampled_textures[v].Load(int3((int(0)).xx, int(0)));
}

