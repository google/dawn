fn countOneBits_690cfc() {
  var res : vec3<u32> = countOneBits(vec3<u32>(1u));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  countOneBits_690cfc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  countOneBits_690cfc();
}

@compute @workgroup_size(1)
fn compute_main() {
  countOneBits_690cfc();
}
