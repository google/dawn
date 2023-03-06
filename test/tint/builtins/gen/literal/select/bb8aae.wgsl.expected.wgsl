fn select_bb8aae() {
  var res : vec4<f32> = select(vec4<f32>(1.0f), vec4<f32>(1.0f), vec4<bool>(true));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_bb8aae();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_bb8aae();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_bb8aae();
}
