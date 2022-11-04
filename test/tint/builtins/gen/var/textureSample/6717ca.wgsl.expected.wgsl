@group(1) @binding(0) var arg_0 : texture_2d_array<f32>;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSample_6717ca() {
  var arg_2 = vec2<f32>(1.0f);
  var arg_3 = 1i;
  var res : vec4<f32> = textureSample(arg_0, arg_1, arg_2, arg_3);
}

@fragment
fn fragment_main() {
  textureSample_6717ca();
}
