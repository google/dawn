fn smoothstep_66e4bd() {
  var res = smoothstep(vec3(2.0), vec3(4.0), vec3(3.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  smoothstep_66e4bd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  smoothstep_66e4bd();
}

@compute @workgroup_size(1)
fn compute_main() {
  smoothstep_66e4bd();
}
