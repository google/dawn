enable f16;

fn max_445169() {
  var arg_0 = vec3<f16>(f16());
  var arg_1 = vec3<f16>(f16());
  var res : vec3<f16> = max(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_445169();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_445169();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_445169();
}
