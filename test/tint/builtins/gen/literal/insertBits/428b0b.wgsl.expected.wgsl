fn insertBits_428b0b() {
  var res : vec3<i32> = insertBits(vec3<i32>(1i), vec3<i32>(1i), 1u, 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  insertBits_428b0b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  insertBits_428b0b();
}

@compute @workgroup_size(1)
fn compute_main() {
  insertBits_428b0b();
}
