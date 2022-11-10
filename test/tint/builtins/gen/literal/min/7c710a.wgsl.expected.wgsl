enable f16;

fn min_7c710a() {
  var res : vec4<f16> = min(vec4<f16>(1.0h), vec4<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_7c710a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_7c710a();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_7c710a();
}
