intrinsics/gen/modf/51e4c6.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec2<f32> = modf(vec2<f32>(), &arg_1);
                       ^^^^

fn modf_51e4c6() {
  var arg_1 : vec2<f32>;
  var res : vec2<f32> = modf(vec2<f32>(), &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  modf_51e4c6();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  modf_51e4c6();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  modf_51e4c6();
}
