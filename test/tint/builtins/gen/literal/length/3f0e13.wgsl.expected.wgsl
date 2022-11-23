enable f16;

fn length_3f0e13() {
  var res : f16 = length(vec2<f16>(0.0h));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  length_3f0e13();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  length_3f0e13();
}

@compute @workgroup_size(1)
fn compute_main() {
  length_3f0e13();
}
