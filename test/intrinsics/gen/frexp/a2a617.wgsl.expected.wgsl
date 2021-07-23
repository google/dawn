intrinsics/gen/frexp/a2a617.wgsl:29:18 warning: use of deprecated intrinsic
  var res: f32 = frexp(1.0, &arg_1);
                 ^^^^^

var<private> arg_1 : i32;

fn frexp_a2a617() {
  var res : f32 = frexp(1.0, &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_a2a617();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_a2a617();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  frexp_a2a617();
}
