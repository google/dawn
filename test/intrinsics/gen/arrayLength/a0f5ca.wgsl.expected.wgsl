[[block]]
struct SB_RO {
  arg_0 : array<f32>;
};

[[group(0), binding(1)]] var<storage, read> sb_ro : SB_RO;

fn arrayLength_a0f5ca() {
  var res : u32 = arrayLength(&(sb_ro.arg_0));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  arrayLength_a0f5ca();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  arrayLength_a0f5ca();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  arrayLength_a0f5ca();
}
