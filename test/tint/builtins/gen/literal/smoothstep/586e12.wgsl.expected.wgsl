enable f16;

fn smoothstep_586e12() {
  var res : f16 = smoothstep(2.0h, 4.0h, 3.0h);
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
