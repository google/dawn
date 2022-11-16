fn sqrt_9c5cbe() {
  const arg_0 = vec2(1.0);
  var res = sqrt(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sqrt_9c5cbe();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sqrt_9c5cbe();
}

@compute @workgroup_size(1)
fn compute_main() {
  sqrt_9c5cbe();
}
