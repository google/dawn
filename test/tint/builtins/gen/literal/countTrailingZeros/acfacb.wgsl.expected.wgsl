fn countTrailingZeros_acfacb() {
  var res : vec3<i32> = countTrailingZeros(vec3<i32>(1));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  countTrailingZeros_acfacb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  countTrailingZeros_acfacb();
}

@compute @workgroup_size(1)
fn compute_main() {
  countTrailingZeros_acfacb();
}
