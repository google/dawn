intrinsics/gen/isNormal/b00ab1.wgsl:28:25 warning: use of deprecated intrinsic
  var res: vec2<bool> = isNormal(vec2<f32>());
                        ^^^^^^^^

fn isNormal_b00ab1() {
  var res : vec2<bool> = isNormal(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  isNormal_b00ab1();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  isNormal_b00ab1();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  isNormal_b00ab1();
}
