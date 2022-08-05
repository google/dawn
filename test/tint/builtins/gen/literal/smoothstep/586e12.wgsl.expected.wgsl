enable f16;

fn smoothstep_586e12() {
  var res : f16 = smoothstep(f16(), f16(), f16());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  smoothstep_586e12();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  smoothstep_586e12();
}

@compute @workgroup_size(1)
fn compute_main() {
  smoothstep_586e12();
}
