intrinsics/gen/isNormal/863dcd.wgsl:28:25 warning: use of deprecated intrinsic
  var res: vec4<bool> = isNormal(vec4<f32>());
                        ^^^^^^^^

fn isNormal_863dcd() {
  var res : vec4<bool> = isNormal(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  isNormal_863dcd();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  isNormal_863dcd();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  isNormal_863dcd();
}
