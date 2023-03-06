@group(1) @binding(0) var arg_0 : texture_multisampled_2d<i32>;

fn textureNumSamples_c1a777() {
  var res : u32 = textureNumSamples(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumSamples_c1a777();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumSamples_c1a777();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumSamples_c1a777();
}
