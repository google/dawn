SKIP: FAILED

void original_clusterfuzz_code() {
}

void more_tests_that_would_fail() {
  float a = 1.47112762928009033203f;
  float b = 0.09966865181922912598f;
  float a = 2.5f;
  float b = 2.5f;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

DXC validation failure:
hlsl.hlsl:7:9: error: redefinition of 'a'
  float a = 2.5f;
        ^
hlsl.hlsl:5:9: note: previous definition is here
  float a = 1.47112762928009033203f;
        ^
hlsl.hlsl:8:9: error: redefinition of 'b'
  float b = 2.5f;
        ^
hlsl.hlsl:6:9: note: previous definition is here
  float b = 0.09966865181922912598f;
        ^

