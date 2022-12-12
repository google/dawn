fn mix_9c2681() {
  var res = mix(vec3(1.0), vec3(1.0), 1.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_9c2681();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_9c2681();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_9c2681();
}
