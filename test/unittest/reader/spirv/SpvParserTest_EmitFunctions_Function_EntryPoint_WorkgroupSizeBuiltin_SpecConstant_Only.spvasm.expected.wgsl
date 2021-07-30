[[override(0)]] let x_3 : u32 = 3u;

[[override(1)]] let x_4 : u32 = 5u;

[[override(2)]] let x_5 : u32 = 7u;

fn comp_main_1() {
  return;
}

[[stage(compute), workgroup_size(3, 5, 7)]]
fn comp_main() {
  comp_main_1();
}
