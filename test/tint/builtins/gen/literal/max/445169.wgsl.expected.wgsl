enable f16;

fn max_445169() {
  var res : vec3<f16> = max(vec3<f16>(1.0h), vec3<f16>(1.0h));
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
