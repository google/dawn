[[group(1), binding(0)]] var arg_0 : texture_3d<f32>;

[[group(1), binding(1)]] var arg_1 : sampler;

fn textureSampleBias_d3fa1b() {
  var res : vec4<f32> = textureSampleBias(arg_0, arg_1, vec3<f32>(), 1.0);
}

[[stage(fragment)]]
fn fragment_main() {
  textureSampleBias_d3fa1b();
}
