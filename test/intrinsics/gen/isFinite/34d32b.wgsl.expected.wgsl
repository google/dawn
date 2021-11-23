intrinsics/gen/isFinite/34d32b.wgsl:28:25 warning: use of deprecated intrinsic
  var res: vec2<bool> = isFinite(vec2<f32>());
                        ^^^^^^^^

fn isFinite_34d32b() {
  var res : vec2<bool> = isFinite(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  isFinite_34d32b();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  isFinite_34d32b();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  isFinite_34d32b();
}
