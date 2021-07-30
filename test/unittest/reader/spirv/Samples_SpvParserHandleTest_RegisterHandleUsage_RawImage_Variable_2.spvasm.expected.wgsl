[[group(0), binding(0)]] var x_20 : texture_1d<f32>;

fn main_1() {
  let x_125 : vec4<f32> = textureLoad(x_20, i32(0u), 0);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
