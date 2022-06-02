fn countOneBits_690cfc() {
  var arg_0 = vec3<u32>();
  var res : vec3<u32> = countOneBits(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  countOneBits_690cfc();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  countOneBits_690cfc();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  countOneBits_690cfc();
}
