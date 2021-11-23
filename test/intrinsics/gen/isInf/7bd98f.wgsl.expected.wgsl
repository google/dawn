intrinsics/gen/isInf/7bd98f.wgsl:28:19 warning: use of deprecated intrinsic
  var res: bool = isInf(1.0);
                  ^^^^^

fn isInf_7bd98f() {
  var res : bool = isInf(1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  isInf_7bd98f();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  isInf_7bd98f();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  isInf_7bd98f();
}
