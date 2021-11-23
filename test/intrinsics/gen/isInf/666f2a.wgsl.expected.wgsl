intrinsics/gen/isInf/666f2a.wgsl:28:25 warning: use of deprecated intrinsic
  var res: vec3<bool> = isInf(vec3<f32>());
                        ^^^^^

fn isInf_666f2a() {
  var res : vec3<bool> = isInf(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  isInf_666f2a();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  isInf_666f2a();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  isInf_666f2a();
}
