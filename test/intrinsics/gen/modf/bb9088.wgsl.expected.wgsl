intrinsics/gen/modf/bb9088.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec3<f32> = modf(vec3<f32>(), &arg_1);
                       ^^^^

var<workgroup> arg_1 : vec3<f32>;

fn modf_bb9088() {
  var res : vec3<f32> = modf(vec3<f32>(), &(arg_1));
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  modf_bb9088();
}
