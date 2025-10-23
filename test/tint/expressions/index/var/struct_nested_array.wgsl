struct S {
  m: i32,
  n: array<u32, 4>,
}

fn f() -> u32 {
  var a = S();
  return a.n[2];
}

@compute @workgroup_size(1)
fn main() {
    _ = f();
}
