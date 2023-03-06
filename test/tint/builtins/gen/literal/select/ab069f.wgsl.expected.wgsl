fn select_ab069f() {
  var res : vec4<i32> = select(vec4<i32>(1i), vec4<i32>(1i), true);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_ab069f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_ab069f();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_ab069f();
}
