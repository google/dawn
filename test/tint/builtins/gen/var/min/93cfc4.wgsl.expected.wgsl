fn min_93cfc4() {
  var arg_0 = vec3<f32>(1.0f);
  var arg_1 = vec3<f32>(1.0f);
  var res : vec3<f32> = min(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_93cfc4();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_93cfc4();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_93cfc4();
}
