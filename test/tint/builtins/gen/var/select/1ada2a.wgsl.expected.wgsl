enable f16;

fn select_1ada2a() {
  var arg_0 = vec3<f16>(1.0h);
  var arg_1 = vec3<f16>(1.0h);
  var arg_2 = true;
  var res : vec3<f16> = select(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_1ada2a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_1ada2a();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_1ada2a();
}
