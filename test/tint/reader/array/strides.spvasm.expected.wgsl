struct tint_padded_array_element {
  @size(8u)
  tint_element : f32,
}

struct tint_padded_array_element_1 {
  @size(128u)
  tint_element_1 : array<array<tint_padded_array_element, 2u>, 3u>,
}

struct S_1 {
  a : array<tint_padded_array_element_1, 4u>,
}

@group(0u) @binding(0u) var<storage, read_write> s : S_1;

@compute @workgroup_size(1u, 1u, 1u)
fn f() {
  _ = s.a;
  _ = s.a[3i].tint_element_1;
  _ = s.a[3i].tint_element_1[2i];
  _ = s.a[3i].tint_element_1[2i][1i].tint_element;
  s.a = array<tint_padded_array_element_1, 4u>();
  s.a[3i].tint_element_1[2i][1i].tint_element = 5.0f;
}
