fn select_1e960b() {
  var arg_0 = vec2<u32>(1u);
  var arg_1 = vec2<u32>(1u);
  var arg_2 = vec2<bool>(true);
  var res : vec2<u32> = select(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_1e960b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_1e960b();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_1e960b();
}
