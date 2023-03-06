fn dot_ba4246() {
  var arg_0 = vec3<f32>(1.0f);
  var arg_1 = vec3<f32>(1.0f);
  var res : f32 = dot(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot_ba4246();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot_ba4246();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_ba4246();
}
