intrinsics/gen/modf/e38ae6.wgsl:29:18 warning: use of deprecated intrinsic
  var res: f32 = modf(1.0, &arg_1);
                 ^^^^

var<workgroup> arg_1 : f32;

fn modf_e38ae6() {
  var res : f32 = modf(1.0, &(arg_1));
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  modf_e38ae6();
}
