fn any_e755c1() {
  var arg_0 = vec3<bool>(true);
  var res : bool = any(arg_0);
  prevent_dce = select(0, 1, all((res == bool())));
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  any_e755c1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  any_e755c1();
}

@compute @workgroup_size(1)
fn compute_main() {
  any_e755c1();
}
