[[block]]
struct S {
  a : i32;
};

[[group(0), binding(0)]]
var<storage> v : [[access(read_write)]] S;

[[stage(compute)]]
fn main() {
  let p : ptr<storage, i32> = &v.a;
  let use : i32 = *p + 1;
}
