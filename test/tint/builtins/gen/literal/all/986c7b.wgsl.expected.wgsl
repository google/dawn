fn all_986c7b() {
  var res : bool = all(vec4<bool>(true));
  prevent_dce = select(0, 1, all((res == bool())));
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  all_986c7b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  all_986c7b();
}

@compute @workgroup_size(1)
fn compute_main() {
  all_986c7b();
}
