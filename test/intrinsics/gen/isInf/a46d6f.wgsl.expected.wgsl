intrinsics/gen/isInf/a46d6f.wgsl:28:25 warning: use of deprecated intrinsic
  var res: vec2<bool> = isInf(vec2<f32>());
                        ^^^^^

fn isInf_a46d6f() {
  var res : vec2<bool> = isInf(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  isInf_a46d6f();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  isInf_a46d6f();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  isInf_a46d6f();
}
