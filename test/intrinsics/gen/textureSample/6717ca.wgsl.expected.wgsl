[[group(1), binding(0)]] var arg_0 : texture_2d_array<f32>;

[[group(1), binding(1)]] var arg_1 : sampler;

fn textureSample_6717ca() {
  var res : vec4<f32> = textureSample(arg_0, arg_1, vec2<f32>(), 1);
}

[[stage(fragment)]]
fn fragment_main() {
  textureSample_6717ca();
}
