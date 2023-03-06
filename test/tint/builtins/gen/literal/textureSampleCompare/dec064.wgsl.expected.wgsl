@group(1) @binding(0) var arg_0 : texture_depth_2d;

@group(1) @binding(1) var arg_1 : sampler_comparison;

fn textureSampleCompare_dec064() {
  var res : f32 = textureSampleCompare(arg_0, arg_1, vec2<f32>(1.0f), 1.0f, vec2<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@fragment
fn fragment_main() {
  textureSampleCompare_dec064();
}
