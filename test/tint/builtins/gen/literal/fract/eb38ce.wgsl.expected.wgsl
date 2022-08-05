enable f16;

fn fract_eb38ce() {
  var res : f16 = fract(f16());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fract_eb38ce();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fract_eb38ce();
}

@compute @workgroup_size(1)
fn compute_main() {
  fract_eb38ce();
}
