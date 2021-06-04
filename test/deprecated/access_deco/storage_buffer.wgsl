
[[block]]
struct SB {
  a : f32;
};

[[group(0), binding(0)]] var<storage> sb : [[access(read_write)]] SB;

[[stage(compute)]]
fn main() {
  var x : f32 = sb.a;
}
