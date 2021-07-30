fn comp_main_1() {
  return;
}

[[stage(compute), workgroup_size(3, 5, 7)]]
fn comp_main() {
  comp_main_1();
}
