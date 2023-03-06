@group(1) @binding(0) var arg_0 : texture_depth_cube_array;

@group(1) @binding(1) var arg_1 : sampler_comparison;

fn textureSampleCompare_a3ca7e() {
  var res : f32 = textureSampleCompare(arg_0, arg_1, vec3<f32>(1.0f), 1i, 1.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@fragment
fn fragment_main() {
  textureSampleCompare_a3ca7e();
}
