fn select_80a9a9() {
  var res : vec3<bool> = select(vec3<bool>(true), vec3<bool>(true), vec3<bool>(true));
  prevent_dce = select(0, 1, all((res == vec3<bool>())));
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_80a9a9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_80a9a9();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_80a9a9();
}
