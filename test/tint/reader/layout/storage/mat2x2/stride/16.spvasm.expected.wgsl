struct tint_padded_array_element {
  @size(16u)
  tint_element : vec2<f32>,
}

struct SSBO_1_1 {
  @align(8u)
  m : array<tint_padded_array_element, 2u>,
}

@group(0u) @binding(0u) var<storage, read_write> ssbo : SSBO_1_1;

@compute @workgroup_size(1u, 1u, 1u)
fn f() {
  let v = ssbo.m;
  let v_1 = mat2x2<f32>(v[0u].tint_element, v[1u].tint_element);
  ssbo.m = array<tint_padded_array_element, 2u>(tint_padded_array_element(v_1[0u]), tint_padded_array_element(v_1[1u]));
}
