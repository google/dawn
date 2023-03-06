fn any_2ab91a() {
  var res : bool = any(true);
  prevent_dce = select(0, 1, all((res == bool())));
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  any_2ab91a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  any_2ab91a();
}

@compute @workgroup_size(1)
fn compute_main() {
  any_2ab91a();
}
