fn smoothstep_aad1db() {
  var res : vec3<f32> = smoothstep(vec3<f32>(), vec3<f32>(), vec3<f32>());
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  smoothstep_aad1db();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  smoothstep_aad1db();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  smoothstep_aad1db();
}
