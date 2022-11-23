enable f16;

fn length_ba16d6() {
  var res : f16 = length(vec3<f16>(0.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  length_ba16d6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  length_ba16d6();
}

@compute @workgroup_size(1)
fn compute_main() {
  length_ba16d6();
}
