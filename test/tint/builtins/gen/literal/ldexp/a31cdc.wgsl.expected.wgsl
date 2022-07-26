fn ldexp_a31cdc() {
  var res : vec3<f32> = ldexp(vec3<f32>(1.0f), vec3<i32>(1));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_a31cdc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_a31cdc();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_a31cdc();
}
