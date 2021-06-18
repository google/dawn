intrinsics/gen/arrayLength/721c9d.wgsl:33:18 warning: use of deprecated intrinsic
  var res: u32 = arrayLength(sb_ro.arg_0);
                 ^^^^^^^^^^^

[[block]]
struct SB_RO {
  arg_0 : array<i32>;
};

[[group(0), binding(1)]] var<storage, read> sb_ro : SB_RO;

fn arrayLength_721c9d() {
  var res : u32 = arrayLength(sb_ro.arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  arrayLength_721c9d();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  arrayLength_721c9d();
}

[[stage(compute)]]
fn compute_main() {
  arrayLength_721c9d();
}
