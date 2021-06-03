[[group(1), binding(0)]] var arg_0 : texture_3d<f32>;

[[group(1), binding(1)]] var arg_1 : sampler;

fn textureSampleGrad_21402b() {
  var res : vec4<f32> = textureSampleGrad(arg_0, arg_1, vec3<f32>(), vec3<f32>(), vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureSampleGrad_21402b();
}

[[stage(fragment)]]
fn fragment_main() {
  textureSampleGrad_21402b();
}

[[stage(compute)]]
fn compute_main() {
  textureSampleGrad_21402b();
}
