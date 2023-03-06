@group(1) @binding(0) var arg_0 : texture_multisampled_2d<f32>;

fn textureLoad_a583c9() {
  var res : vec4<f32> = textureLoad(arg_0, vec2<i32>(1i), 1i);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_a583c9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_a583c9();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_a583c9();
}
