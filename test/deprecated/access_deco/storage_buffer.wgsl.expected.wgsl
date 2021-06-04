deprecated/access_deco/storage_buffer.wgsl:7:26 warning: use of deprecated language feature: declare access with var<storage, read_write> instead of using [[access]] decoration
[[group(0), binding(0)]] var<storage> sb : [[access(read_write)]] SB;
                         ^^^

[[block]]
struct SB {
  a : f32;
};

[[group(0), binding(0)]] var<storage, read_write> sb : SB;

[[stage(compute)]]
fn main() {
  var x : f32 = sb.a;
}
