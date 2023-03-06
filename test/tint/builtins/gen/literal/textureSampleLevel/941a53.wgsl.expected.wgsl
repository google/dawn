@group(1) @binding(0) var arg_0 : texture_depth_cube_array;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSampleLevel_941a53() {
  var res : f32 = textureSampleLevel(arg_0, arg_1, vec3<f32>(1.0f), 1u, 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureSampleLevel_941a53();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureSampleLevel_941a53();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureSampleLevel_941a53();
}
