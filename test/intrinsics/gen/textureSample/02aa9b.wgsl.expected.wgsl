[[group(1), binding(0)]] var arg_0 : texture_2d_array<f32>;

[[group(1), binding(1)]] var arg_1 : sampler;

fn textureSample_02aa9b() {
  var res : vec4<f32> = textureSample(arg_0, arg_1, vec2<f32>(), 1, vec2<i32>());
}

[[stage(fragment)]]
fn fragment_main() {
  textureSample_02aa9b();
}
