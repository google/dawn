@group(1) @binding(0) var arg_0 : texture_depth_multisampled_2d;

fn textureLoad_6925bc() {
  var res : f32 = textureLoad(arg_0, vec2<i32>(1i), 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_6925bc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_6925bc();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_6925bc();
}
