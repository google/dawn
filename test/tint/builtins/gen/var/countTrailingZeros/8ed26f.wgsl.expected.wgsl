fn countTrailingZeros_8ed26f() {
  var arg_0 = vec3<u32>(1u);
  var res : vec3<u32> = countTrailingZeros(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  countTrailingZeros_8ed26f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  countTrailingZeros_8ed26f();
}

@compute @workgroup_size(1)
fn compute_main() {
  countTrailingZeros_8ed26f();
}
