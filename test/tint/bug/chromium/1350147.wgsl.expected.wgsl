fn original_clusterfuzz_code() {
  atan2(1, 0.1);
}

fn more_tests_that_would_fail() {
  {
    let a = atan2(1, 0.1);
    let b = atan2(0.1, 1);
  }
  {
    let a = (1 + 1.5);
    let b = (1.5 + 1);
  }
  {
    atan2(1, 0.1);
    atan2(0.1, 1);
  }
}
