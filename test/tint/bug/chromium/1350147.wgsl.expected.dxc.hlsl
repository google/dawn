[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void original_clusterfuzz_code() {
}

void more_tests_that_would_fail() {
  {
    const float a = 1.471127629f;
    const float b = 0.099668652f;
  }
  {
    const float a = 2.5f;
    const float b = 2.5f;
  }
  {
  }
}
