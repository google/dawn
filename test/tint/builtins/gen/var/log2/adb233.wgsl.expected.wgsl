fn log2_adb233() {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = log2(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  log2_adb233();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  log2_adb233();
}

@compute @workgroup_size(1)
fn compute_main() {
  log2_adb233();
}
