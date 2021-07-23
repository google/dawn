intrinsics/gen/modf/353f7d.wgsl:29:18 warning: use of deprecated intrinsic
  var res: f32 = modf(1.0, &arg_1);
                 ^^^^

fn modf_353f7d() {
  var arg_1 : f32;
  var res : f32 = modf(1.0, &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  modf_353f7d();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  modf_353f7d();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  modf_353f7d();
}
