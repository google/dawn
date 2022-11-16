fn smoothstep_392c19() {
  var res : vec2<f32> = smoothstep(vec2<f32>(2.0f), vec2<f32>(4.0f), vec2<f32>(3.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  smoothstep_392c19();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  smoothstep_392c19();
}

@compute @workgroup_size(1)
fn compute_main() {
  smoothstep_392c19();
}
