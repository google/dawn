fn select_3c25ce() {
  var res : vec3<bool> = select(vec3<bool>(true), vec3<bool>(true), true);
  prevent_dce = select(0, 1, all((res == vec3<bool>())));
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_3c25ce();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_3c25ce();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_3c25ce();
}
