fn max_b1b73a() {
  var res : vec3<u32> = max(vec3<u32>(1u), vec3<u32>(1u));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_b1b73a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_b1b73a();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_b1b73a();
}
