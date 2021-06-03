[[group(1), binding(0)]] var arg_0 : texture_3d<f32>;

[[group(1), binding(1)]] var arg_1 : sampler;

fn textureSampleLevel_abfcc0() {
  var res : vec4<f32> = textureSampleLevel(arg_0, arg_1, vec3<f32>(), 1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureSampleLevel_abfcc0();
}

[[stage(fragment)]]
fn fragment_main() {
  textureSampleLevel_abfcc0();
}

[[stage(compute)]]
fn compute_main() {
  textureSampleLevel_abfcc0();
}
