fn comp_main_1() {
  return;
}

[[stage(compute), workgroup_size(2, 4, 8)]]
fn comp_main() {
  comp_main_1();
}
