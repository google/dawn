@group(1) @binding(0) var arg_0 : texture_depth_cube_array;

@group(1) @binding(1) var arg_1 : sampler_comparison;

fn textureSampleCompare_1912e5() {
  var res : f32 = textureSampleCompare(arg_0, arg_1, vec3<f32>(), 1u, 1.0f);
}

@fragment
fn fragment_main() {
  textureSampleCompare_1912e5();
}
