var<private> I : i32;

[[stage(compute)]]
fn main() {
  let i : i32 = I;
  let use : i32 = (i + 1);
}
