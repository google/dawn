fn smoothstep_6c4975() {
  var res : f32 = smoothstep(1.0, 1.0, 1.0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  smoothstep_6c4975();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  smoothstep_6c4975();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  smoothstep_6c4975();
}
