intrinsics/gen/modf/a128ab.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec2<f32> = modf(vec2<f32>(), &arg_1);
                       ^^^^

var<workgroup> arg_1 : vec2<f32>;

fn modf_a128ab() {
  var res : vec2<f32> = modf(vec2<f32>(), &(arg_1));
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  modf_a128ab();
}
