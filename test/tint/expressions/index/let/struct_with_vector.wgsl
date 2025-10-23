struct S {
  m: i32,
  n: vec3u,
}

fn f() -> u32 {
  let a = S();
  return a.n[2];
}

@compute @workgroup_size(1)
fn main() {
    _ = f();
}
