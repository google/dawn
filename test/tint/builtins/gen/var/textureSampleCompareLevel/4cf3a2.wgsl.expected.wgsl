@group(1) @binding(0) var arg_0 : texture_depth_cube_array;

@group(1) @binding(1) var arg_1 : sampler_comparison;

fn textureSampleCompareLevel_4cf3a2() {
  var arg_2 = vec3<f32>(1.0f);
  var arg_3 = 1i;
  var arg_4 = 1.0f;
  var res : f32 = textureSampleCompareLevel(arg_0, arg_1, arg_2, arg_3, arg_4);
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
