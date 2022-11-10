enable f16;

fn dot_d0d179() {
  var res : f16 = dot(vec4<f16>(1.0h), vec4<f16>(1.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot_d0d179();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot_d0d179();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_d0d179();
}
