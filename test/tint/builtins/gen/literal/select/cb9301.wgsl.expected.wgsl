fn select_cb9301() {
  var res : vec2<bool> = select(vec2<bool>(true), vec2<bool>(true), vec2<bool>(true));
  prevent_dce = select(0, 1, all((res == vec2<bool>())));
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_cb9301();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_cb9301();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_cb9301();
}
