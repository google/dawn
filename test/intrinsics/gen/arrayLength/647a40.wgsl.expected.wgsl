intrinsics/gen/arrayLength/647a40.wgsl:33:18 warning: use of deprecated intrinsic
  var res: u32 = arrayLength(sb.arg_0);
                 ^^^^^^^^^^^

[[block]]
struct SB {
  arg_0 : array<u32>;
};

[[group(0), binding(0)]] var<storage, read> sb : SB;

fn arrayLength_647a40() {
  var res : u32 = arrayLength(sb.arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  arrayLength_647a40();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  arrayLength_647a40();
}

[[stage(compute)]]
fn compute_main() {
  arrayLength_647a40();
}
