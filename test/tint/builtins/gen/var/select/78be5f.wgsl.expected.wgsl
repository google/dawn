fn select_78be5f() {
  var arg_0 = vec3<f32>(1.0f);
  var arg_1 = vec3<f32>(1.0f);
  var arg_2 = true;
  var res : vec3<f32> = select(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_78be5f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_78be5f();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_78be5f();
}
