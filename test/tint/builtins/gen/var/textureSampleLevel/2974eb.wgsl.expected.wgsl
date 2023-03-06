@group(1) @binding(0) var arg_0 : texture_depth_2d_array;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSampleLevel_2974eb() {
  var arg_2 = vec2<f32>(1.0f);
  var arg_3 = 1i;
  var arg_4 = 1u;
  var res : f32 = textureSampleLevel(arg_0, arg_1, arg_2, arg_3, arg_4);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureSampleLevel_2974eb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureSampleLevel_2974eb();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureSampleLevel_2974eb();
}
