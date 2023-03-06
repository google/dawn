@group(1) @binding(0) var arg_0 : texture_3d<f32>;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSampleLevel_dcbecb() {
  var arg_2 = vec3<f32>(1.0f);
  var arg_3 = 1.0f;
  const arg_4 = vec3<i32>(1i);
  var res : vec4<f32> = textureSampleLevel(arg_0, arg_1, arg_2, arg_3, arg_4);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureSampleLevel_dcbecb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureSampleLevel_dcbecb();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureSampleLevel_dcbecb();
}
