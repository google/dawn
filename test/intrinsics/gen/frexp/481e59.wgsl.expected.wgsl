intrinsics/gen/frexp/481e59.wgsl:29:18 warning: use of deprecated intrinsic
  var res: f32 = frexp(1.0, &arg_1);
                 ^^^^^

fn frexp_481e59() {
  var arg_1 : i32;
  var res : f32 = frexp(1.0, &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_481e59();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_481e59();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  frexp_481e59();
}
