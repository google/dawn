#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
void original_clusterfuzz_code() {
}

void more_tests_that_would_fail() {
  {
    float a = 1.471127629f;
    float b = 0.099668652f;
  }
  {
    float a = 2.5f;
    float b = 2.5f;
  }
  {
  }
}

