@group(1) @binding(0) var arg_0 : texture_depth_2d;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSample_0dff6c() {
  var res : f32 = textureSample(arg_0, arg_1, vec2<f32>(1.0f), vec2<i32>(1i));
}

@fragment
fn fragment_main() {
  textureSample_0dff6c();
}
