[[block]]
struct SB {
  arg_0 : array<i32>;
};

[[group(0), binding(0)]] var<storage, read> sb : SB;

fn arrayLength_721c9d() {
  var res : u32 = arrayLength(sb.arg_0);
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
