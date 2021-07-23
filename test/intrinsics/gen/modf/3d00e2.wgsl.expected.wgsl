intrinsics/gen/modf/3d00e2.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec4<f32> = modf(vec4<f32>(), &arg_1);
                       ^^^^

var<private> arg_1 : vec4<f32>;

fn modf_3d00e2() {
  var res : vec4<f32> = modf(vec4<f32>(), &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  modf_3d00e2();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  modf_3d00e2();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  modf_3d00e2();
}
