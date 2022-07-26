fn max_b1b73a() {
  var arg_0 = vec3<u32>(1u);
  var arg_1 = vec3<u32>(1u);
  var res : vec3<u32> = max(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_b1b73a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_b1b73a();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_b1b73a();
}
