var<private> I = 0i;

fn main_1() {
  let x_9 = I;
  let x_11 = (x_9 + 1i);
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
