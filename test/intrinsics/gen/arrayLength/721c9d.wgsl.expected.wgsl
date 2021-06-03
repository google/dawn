[[block]]
struct SB {
  arg_0 : array<i32>;
};

[[group(0), binding(0)]] var<storage> sb : [[access(read)]] SB;

fn arrayLength_721c9d() {
  var res : u32 = arrayLength(sb.arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  arrayLength_721c9d();
}

[[stage(fragment)]]
fn fragment_main() {
  arrayLength_721c9d();
}

[[stage(compute)]]
fn compute_main() {
  arrayLength_721c9d();
}
