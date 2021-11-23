[[block]]
struct SB {
  x : i32;
  arr : array<i32>;
};

[[group(0), binding(0)]] var<storage, read> sb : SB;

[[stage(compute), workgroup_size(1)]]
fn main() {
  var a : u32 = arrayLength(&sb.arr);
  let p = &sb;
  let sb2 = p;
  var b : u32 = arrayLength(&((*sb2).arr));
}
