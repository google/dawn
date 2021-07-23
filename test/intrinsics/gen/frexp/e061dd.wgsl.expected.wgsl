intrinsics/gen/frexp/e061dd.wgsl:29:18 warning: use of deprecated intrinsic
  var res: f32 = frexp(1.0, &arg_1);
                 ^^^^^

fn frexp_e061dd() {
  var arg_1 : i32;
  var res : f32 = frexp(1.0, &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_e061dd();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_e061dd();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  frexp_e061dd();
}
