fn main_1() {
  loop {
    var x_600 : f32;
    if (true) {
      break;
    }
    if (true) {
      x_600 = 50.0;
    }
    break;

    continuing {
      let x_82 : u32 = u32(x_600);
    }
  }
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
