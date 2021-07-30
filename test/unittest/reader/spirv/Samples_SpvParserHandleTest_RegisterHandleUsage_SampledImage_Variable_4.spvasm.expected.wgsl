[[group(0), binding(0)]] var x_10 : sampler_comparison;

[[group(0), binding(1)]] var x_20 : texture_depth_2d;

fn main_1() {
  let x_131 : f32 = textureSampleCompare(x_20, x_10, vec2<f32>(0.0, 0.0), 0.200000003);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
