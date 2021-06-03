[[group(1), binding(0)]] var arg_0 : texture_2d_array<f32>;

[[group(1), binding(1)]] var arg_1 : sampler;

fn textureSampleGrad_872f00() {
  var res : vec4<f32> = textureSampleGrad(arg_0, arg_1, vec2<f32>(), 1, vec2<f32>(), vec2<f32>(), vec2<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureSampleGrad_872f00();
}

[[stage(fragment)]]
fn fragment_main() {
  textureSampleGrad_872f00();
}

[[stage(compute)]]
fn compute_main() {
  textureSampleGrad_872f00();
}
