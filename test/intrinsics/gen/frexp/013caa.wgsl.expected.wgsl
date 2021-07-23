intrinsics/gen/frexp/013caa.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec4<f32> = frexp(vec4<f32>(), &arg_1);
                       ^^^^^

fn frexp_013caa() {
  var arg_1 : vec4<i32>;
  var res : vec4<f32> = frexp(vec4<f32>(), &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_013caa();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_013caa();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  frexp_013caa();
}
