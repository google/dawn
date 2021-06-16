deprecated/access_deco/storage_texture.wgsl:1:84 warning: use of deprecated language feature: access control is expected as last parameter of storage textures
[[group(0), binding(0)]] var tex : [[access(write)]] texture_storage_2d<rgba32float>;
                                                                                   ^

RWTexture2D<float4> tex : register(u0, space0);

[numthreads(1, 1, 1)]
void main() {
  int2 tint_tmp;
  tex.GetDimensions(tint_tmp.x, tint_tmp.y);
  int2 x = tint_tmp;
  return;
}
