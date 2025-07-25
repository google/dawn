struct st_ty {
  /* @offset(0) */
  field0 : u32,
}

fn main_1() {
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
