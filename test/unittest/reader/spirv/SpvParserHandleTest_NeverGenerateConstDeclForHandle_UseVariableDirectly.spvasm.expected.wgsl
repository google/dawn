[[group(0), binding(0)]] var x_2 : texture_2d<f32>;

[[group(0), binding(1)]] var x_3 : sampler;

fn main_1() {
  var var_1 : vec4<f32>;
  let x_22 : vec4<f32> = textureSample(x_2, x_3, vec2<f32>(0.0, 0.0));
  let x_26 : vec4<f32> = textureSample(x_2, x_3, vec2<f32>(0.0, 0.0));
  var_1 = (x_22 + x_26);
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
