intrinsics/gen/isFinite/f31987.wgsl:28:25 warning: use of deprecated intrinsic
  var res: vec4<bool> = isFinite(vec4<f32>());
                        ^^^^^^^^

fn isFinite_f31987() {
  var res : vec4<bool> = isFinite(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  isFinite_f31987();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  isFinite_f31987();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  isFinite_f31987();
}
