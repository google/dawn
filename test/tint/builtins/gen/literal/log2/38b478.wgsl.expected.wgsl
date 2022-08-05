enable f16;

fn log2_38b478() {
  var res : vec3<f16> = log2(vec3<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  log2_38b478();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  log2_38b478();
}

@compute @workgroup_size(1)
fn compute_main() {
  log2_38b478();
}
