[[group(0), binding(0)]] var x_20 : texture_storage_2d<rg32float, read>;

fn main_1() {
  let x_125 : vec2<u32> = vec2<u32>(textureDimensions(x_20));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
