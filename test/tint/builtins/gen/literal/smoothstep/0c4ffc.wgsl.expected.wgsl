fn smoothstep_0c4ffc() {
  var res = smoothstep(vec4(2.0), vec4(4.0), vec4(3.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  smoothstep_0c4ffc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  smoothstep_0c4ffc();
}

@compute @workgroup_size(1)
fn compute_main() {
  smoothstep_0c4ffc();
}
