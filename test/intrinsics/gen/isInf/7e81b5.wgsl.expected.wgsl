intrinsics/gen/isInf/7e81b5.wgsl:28:25 warning: use of deprecated intrinsic
  var res: vec4<bool> = isInf(vec4<f32>());
                        ^^^^^

fn isInf_7e81b5() {
  var res : vec4<bool> = isInf(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  isInf_7e81b5();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  isInf_7e81b5();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  isInf_7e81b5();
}
