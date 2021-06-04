deprecated/access_deco/storage_texture.wgsl:1:84 warning: use of deprecated language feature: access control is expected as last parameter of storage textures
[[group(0), binding(0)]] var tex : [[access(write)]] texture_storage_2d<rgba32float>;
                                                                                   ^

[[group(0), binding(0)]] var tex : texture_storage_2d<rgba32float, write>;

[[stage(compute)]]
fn main() {
  var x : vec2<i32> = textureDimensions(tex);
}
