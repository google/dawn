[[block]] struct S { a : i32; };
[[group(0), binding(0)]] var<storage> buf : [[access(read_write)]] S;

[[stage(compute)]] fn main() {
  let p : ptr<storage, i32> = &buf.a;
  *p = 12;
}
