[[block]]
struct SB_RO {
  arg_0 : array<u32>;
};

[[group(0), binding(1)]] var<storage, read> sb_ro : SB_RO;

fn arrayLength_cfca0a() {
  var res : u32 = arrayLength(&(sb_ro.arg_0));
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  arrayLength_cfca0a();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  arrayLength_cfca0a();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  arrayLength_cfca0a();
}
