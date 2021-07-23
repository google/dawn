intrinsics/gen/frexp/2052e9.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec4<f32> = frexp(vec4<f32>(), &arg_1);
                       ^^^^^

fn frexp_2052e9() {
  var arg_1 : vec4<i32>;
  var res : vec4<f32> = frexp(vec4<f32>(), &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_2052e9();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_2052e9();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  frexp_2052e9();
}
