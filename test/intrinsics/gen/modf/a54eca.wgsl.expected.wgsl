intrinsics/gen/modf/a54eca.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec2<f32> = modf(vec2<f32>(), &arg_1);
                       ^^^^

fn modf_a54eca() {
  var arg_1 : vec2<f32>;
  var res : vec2<f32> = modf(vec2<f32>(), &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  modf_a54eca();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  modf_a54eca();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  modf_a54eca();
}
