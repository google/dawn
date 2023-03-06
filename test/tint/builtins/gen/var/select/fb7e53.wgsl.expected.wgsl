fn select_fb7e53() {
  var arg_0 = vec2<bool>(true);
  var arg_1 = vec2<bool>(true);
  var arg_2 = true;
  var res : vec2<bool> = select(arg_0, arg_1, arg_2);
  prevent_dce = select(0, 1, all((res == vec2<bool>())));
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_fb7e53();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_fb7e53();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_fb7e53();
}
