fn dot_97c7ee() {
  var res : u32 = dot(vec2<u32>(), vec2<u32>());
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
