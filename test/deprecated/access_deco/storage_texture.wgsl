[[group(0), binding(0)]] var tex : [[access(write)]] texture_storage_2d<rgba32float>;

[[stage(compute)]]
fn main() {
  var x : vec2<i32> = textureDimensions(tex);
}
