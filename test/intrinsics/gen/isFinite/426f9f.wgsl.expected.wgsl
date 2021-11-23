intrinsics/gen/isFinite/426f9f.wgsl:28:19 warning: use of deprecated intrinsic
  var res: bool = isFinite(1.0);
                  ^^^^^^^^

fn isFinite_426f9f() {
  var res : bool = isFinite(1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  isFinite_426f9f();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  isFinite_426f9f();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  isFinite_426f9f();
}
