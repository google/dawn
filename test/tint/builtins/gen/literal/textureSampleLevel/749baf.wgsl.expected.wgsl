@group(1) @binding(0) var arg_0 : texture_depth_2d;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSampleLevel_749baf() {
  var res : f32 = textureSampleLevel(arg_0, arg_1, vec2<f32>(1.0f), 1i, vec2<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureSampleLevel_749baf();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureSampleLevel_749baf();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureSampleLevel_749baf();
}
