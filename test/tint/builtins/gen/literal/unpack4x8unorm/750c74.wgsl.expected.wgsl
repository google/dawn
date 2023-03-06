fn unpack4x8unorm_750c74() {
  var res : vec4<f32> = unpack4x8unorm(1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  unpack4x8unorm_750c74();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  unpack4x8unorm_750c74();
}

@compute @workgroup_size(1)
fn compute_main() {
  unpack4x8unorm_750c74();
}
