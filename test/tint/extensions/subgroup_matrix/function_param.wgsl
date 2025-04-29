enable chromium_experimental_subgroup_matrix;

struct S {
  l: subgroup_matrix_left<f32, 8, 8>,
  r: subgroup_matrix_right<f32, 8, 8>,
}

struct S_Nested {
  s: S,
}

@group(0) @binding(0)
var<storage, read_write> buffer: array<f32>;

fn foo(
  m: subgroup_matrix_left<f32, 8, 8>,
  m_array: array<subgroup_matrix_left<f32, 8, 8>, 4>,
  m_nested_array: array<array<subgroup_matrix_left<f32, 8, 8>, 4>, 4>,
  m_struct: S,
  m_nested_struct: S_Nested,
) {
  subgroupMatrixStore(&buffer, 0, m, false, 64);
  subgroupMatrixStore(&buffer, 0, m_array[0], false, 64);
  subgroupMatrixStore(&buffer, 0, m_nested_array[1][2], false, 64);
  subgroupMatrixStore(&buffer, 0, m_struct.l, false, 64);
  subgroupMatrixStore(&buffer, 0, m_nested_struct.s.r, false, 64);
}

@compute @workgroup_size(64)
fn main() {
  var m: subgroup_matrix_left<f32, 8, 8>;
  var m_array: array<subgroup_matrix_left<f32, 8, 8>, 4>;
  var m_nested_array: array<array<subgroup_matrix_left<f32, 8, 8>, 4>, 4>;
  var m_struct: S;
  var m_nested_struct: S_Nested;
  foo(m, m_array, m_nested_array, m_struct, m_nested_struct);
}
