fn countLeadingZeros_ab6345() {
  var res : vec3<u32> = countLeadingZeros(vec3<u32>(1u));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  countLeadingZeros_ab6345();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  countLeadingZeros_ab6345();
}

@compute @workgroup_size(1)
fn compute_main() {
  countLeadingZeros_ab6345();
}
