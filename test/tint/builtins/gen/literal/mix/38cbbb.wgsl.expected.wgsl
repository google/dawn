enable f16;

fn mix_38cbbb() {
  var res : f16 = mix(f16(), f16(), f16());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_38cbbb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_38cbbb();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_38cbbb();
}
