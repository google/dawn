fn select_bb8aae() {
  var arg_0 = vec4<f32>(1.0f);
  var arg_1 = vec4<f32>(1.0f);
  var arg_2 = vec4<bool>(true);
  var res : vec4<f32> = select(arg_0, arg_1, arg_2);
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
