[[group(1), binding(0)]] var arg_0 : external_texture;

[[group(1), binding(1)]] var arg_1 : sampler;

fn textureSample_bb81ce() {
  var res : vec4<f32> = textureSample(arg_0, arg_1, vec2<f32>());
}

[[stage(fragment)]]
fn fragment_main() {
  textureSample_bb81ce();
}
