[[stage(fragment)]]
fn main() {
  loop {
    if (false) {
    } else {
      break;
    }

    continuing {
       if (true) {
       } else {
        break;
       }
    }
  }
}
