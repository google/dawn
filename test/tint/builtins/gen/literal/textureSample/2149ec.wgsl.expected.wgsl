@group(1) @binding(0) var arg_0 : texture_3d<f32>;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSample_2149ec() {
  var res : vec4<f32> = textureSample(arg_0, arg_1, vec3<f32>(), vec3<i32>());
}

@fragment
fn fragment_main() {
  textureSample_2149ec();
}
