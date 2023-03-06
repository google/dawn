@group(1) @binding(0) var arg_0 : texture_depth_cube_array;

@group(1) @binding(1) var arg_1 : sampler_comparison;

fn textureSampleCompareLevel_4cf3a2() {
  var res : f32 = textureSampleCompareLevel(arg_0, arg_1, vec3<f32>(1.0f), 1i, 1.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureSampleCompareLevel_4cf3a2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureSampleCompareLevel_4cf3a2();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureSampleCompareLevel_4cf3a2();
}
