fn countOneBits_690cfc() {
  var res : vec3<u32> = countOneBits(vec3<u32>());
}

[[stage(vertex)]]
fn vertex_main() {
  countOneBits_690cfc();
}

[[stage(fragment)]]
fn fragment_main() {
  countOneBits_690cfc();
}

[[stage(compute)]]
fn compute_main() {
  countOneBits_690cfc();
}
