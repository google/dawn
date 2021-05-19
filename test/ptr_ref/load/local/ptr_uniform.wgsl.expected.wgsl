[[block]]
struct S {
  a : i32;
};

[[group(0), binding(0)]] var<uniform> v : S;

[[stage(compute)]]
fn main() {
  let p : ptr<uniform, i32> = &(v.a);
  let use : i32 = (*(p) + 1);
}
