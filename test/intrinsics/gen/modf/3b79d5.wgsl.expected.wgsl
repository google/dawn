intrinsics/gen/modf/3b79d5.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec3<f32> = modf(vec3<f32>(), &arg_1);
                       ^^^^

fn modf_3b79d5() {
  var arg_1 : vec3<f32>;
  var res : vec3<f32> = modf(vec3<f32>(), &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  modf_3b79d5();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  modf_3b79d5();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  modf_3b79d5();
}
