fn dot_e994c7() {
  var res : u32 = dot(vec4<u32>(1u), vec4<u32>(1u));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot_e994c7();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot_e994c7();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_e994c7();
}
