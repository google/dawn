fn dot_97c7ee() {
  var arg_0 = vec2<u32>(1u);
  var arg_1 = vec2<u32>(1u);
  var res : u32 = dot(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot_97c7ee();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot_97c7ee();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_97c7ee();
}
