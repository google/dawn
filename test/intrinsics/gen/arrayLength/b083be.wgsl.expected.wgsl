[[block]]
struct SB {
  arg_0 : array<f32>;
};

[[group(0), binding(0)]] var<storage> sb : [[access(read)]] SB;

fn arrayLength_b083be() {
  var res : u32 = arrayLength(sb.arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  arrayLength_b083be();
}

[[stage(fragment)]]
fn fragment_main() {
  arrayLength_b083be();
}

[[stage(compute)]]
fn compute_main() {
  arrayLength_b083be();
}
