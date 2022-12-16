fn ldexp_593ff3() {
  var res : vec3<f32> = ldexp(vec3<f32>(1.0f), vec3(1));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_593ff3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_593ff3();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_593ff3();
}
