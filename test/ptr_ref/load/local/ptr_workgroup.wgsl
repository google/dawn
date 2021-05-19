var<workgroup> i : i32;

[[stage(compute)]]
fn main() {
  i = 123;
  let p : ptr<workgroup, i32> = &i;
  let use : i32 = *p + 1;
}
