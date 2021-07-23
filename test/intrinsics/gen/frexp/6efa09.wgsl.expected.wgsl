intrinsics/gen/frexp/6efa09.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec3<f32> = frexp(vec3<f32>(), &arg_1);
                       ^^^^^

var<private> arg_1 : vec3<i32>;

fn frexp_6efa09() {
  var res : vec3<f32> = frexp(vec3<f32>(), &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_6efa09();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_6efa09();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  frexp_6efa09();
}
