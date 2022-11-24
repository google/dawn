fn inverseSqrt_6d0783() {
  const arg_0 = vec3(1.0);
  var res = inverseSqrt(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  inverseSqrt_6d0783();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  inverseSqrt_6d0783();
}

@compute @workgroup_size(1)
fn compute_main() {
  inverseSqrt_6d0783();
}
