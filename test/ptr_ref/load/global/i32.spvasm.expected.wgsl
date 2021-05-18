var<private> I : i32 = 0;

[[stage(compute)]]
fn main() {
  let x_9 : i32 = I;
  let x_11 : i32 = (x_9 + 1);
  return;
}
