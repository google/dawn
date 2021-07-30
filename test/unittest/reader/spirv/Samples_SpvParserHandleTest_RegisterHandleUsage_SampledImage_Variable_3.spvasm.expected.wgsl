[[group(0), binding(0)]] var x_10 : sampler;

[[group(0), binding(1)]] var x_20 : texture_2d<f32>;

fn main_1() {
  let x_131 : vec4<f32> = textureSampleLevel(x_20, x_10, vec2<f32>(0.0, 0.0), 0.0);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
