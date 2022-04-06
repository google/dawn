fn smoothstep_40864c() {
  var res : vec4<f32> = smoothstep(vec4<f32>(), vec4<f32>(), vec4<f32>());
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  smoothstep_40864c();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  smoothstep_40864c();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  smoothstep_40864c();
}
