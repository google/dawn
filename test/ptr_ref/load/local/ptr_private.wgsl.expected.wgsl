var<private> i : i32 = 123;

[[stage(compute), workgroup_size(1)]]
fn main() {
  let p : ptr<private, i32> = &(i);
  let use : i32 = (*(p) + 1);
}
