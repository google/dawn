intrinsics/gen/frexp/15edf3.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec2<f32> = frexp(vec2<f32>(), &arg_1);
                       ^^^^^

fn frexp_15edf3() {
  var arg_1 : vec2<i32>;
  var res : vec2<f32> = frexp(vec2<f32>(), &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_15edf3();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_15edf3();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  frexp_15edf3();
}
