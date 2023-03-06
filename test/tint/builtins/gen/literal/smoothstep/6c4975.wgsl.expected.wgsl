fn smoothstep_6c4975() {
  var res : f32 = smoothstep(2.0f, 4.0f, 3.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  smoothstep_6c4975();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  smoothstep_6c4975();
}

@compute @workgroup_size(1)
fn compute_main() {
  smoothstep_6c4975();
}
