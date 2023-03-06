fn min_a45171() {
  var arg_0 = vec3<i32>(1i);
  var arg_1 = vec3<i32>(1i);
  var res : vec3<i32> = min(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_a45171();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_a45171();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_a45171();
}
