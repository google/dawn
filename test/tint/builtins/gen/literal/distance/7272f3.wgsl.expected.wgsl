enable f16;

fn distance_7272f3() {
  var res : f16 = distance(vec4<f16>(f16()), vec4<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  distance_7272f3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  distance_7272f3();
}

@compute @workgroup_size(1)
fn compute_main() {
  distance_7272f3();
}
