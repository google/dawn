intrinsics/gen/frexp/b9e4de.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec3<f32> = frexp(vec3<f32>(), &arg_1);
                       ^^^^^

fn frexp_b9e4de() {
  var arg_1 : vec3<i32>;
  var res : vec3<f32> = frexp(vec3<f32>(), &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  frexp_b9e4de();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_b9e4de();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  frexp_b9e4de();
}
