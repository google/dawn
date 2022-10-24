@group(1) @binding(0) var arg_0 : texture_depth_2d_array;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSample_4703d0() {
  var res : f32 = textureSample(arg_0, arg_1, vec2<f32>(), 1u, vec2<i32>());
}

@fragment
fn fragment_main() {
  textureSample_4703d0();
}
