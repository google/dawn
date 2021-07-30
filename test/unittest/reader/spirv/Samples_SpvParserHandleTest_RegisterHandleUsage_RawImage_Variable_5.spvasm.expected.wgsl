[[group(0), binding(0)]] var x_20 : texture_2d<f32>;

fn main_1() {
  let x_125 : u32 = u32(textureNumLevels(x_20));
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
