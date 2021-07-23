intrinsics/gen/frexp/5a141e.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec3<f32> = frexp(vec3<f32>(), &arg_1);
                       ^^^^^

fn frexp_5a141e() {
  var arg_1 : vec3<i32>;
  var res : vec3<f32> = frexp(vec3<f32>(), &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_5a141e();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_5a141e();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  frexp_5a141e();
}
