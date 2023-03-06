@group(1) @binding(0) var arg_0 : texture_depth_2d;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSample_38bbb9() {
  var arg_2 = vec2<f32>(1.0f);
  var res : f32 = textureSample(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@fragment
fn fragment_main() {
  textureSample_38bbb9();
}
