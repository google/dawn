fn insertBits_428b0b() {
  var res : vec3<i32> = insertBits(vec3<i32>(1), vec3<i32>(1), 1u, 1u);
}

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
