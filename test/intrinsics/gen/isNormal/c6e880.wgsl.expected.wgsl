intrinsics/gen/isNormal/c6e880.wgsl:28:19 warning: use of deprecated intrinsic
  var res: bool = isNormal(1.0);
                  ^^^^^^^^

fn isNormal_c6e880() {
  var res : bool = isNormal(1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  isNormal_c6e880();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  isNormal_c6e880();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  isNormal_c6e880();
}
