intrinsics/gen/modf/9cecfc.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec3<f32> = modf(vec3<f32>(), &arg_1);
                       ^^^^

var<private> arg_1 : vec3<f32>;

fn modf_9cecfc() {
  var res : vec3<f32> = modf(vec3<f32>(), &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  modf_9cecfc();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  modf_9cecfc();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  modf_9cecfc();
}
