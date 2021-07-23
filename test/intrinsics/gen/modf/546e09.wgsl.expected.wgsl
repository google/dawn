intrinsics/gen/modf/546e09.wgsl:29:18 warning: use of deprecated intrinsic
  var res: f32 = modf(1.0, &arg_1);
                 ^^^^

fn modf_546e09() {
  var arg_1 : f32;
  var res : f32 = modf(1.0, &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  modf_546e09();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  modf_546e09();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  modf_546e09();
}
