enable chromium_experimental_subgroup_matrix;

struct S {
  l : subgroup_matrix_left<f32, 8, 8>,
  r : subgroup_matrix_right<f32, 8, 8>,
}

struct S_Nested {
  s : S,
}

@group(0) @binding(0) var<storage, read_write> buffer : array<f32>;

@compute @workgroup_size(64)
fn main() {
  var m : subgroup_matrix_left<f32, 8, 8>;
  var m_array : array<subgroup_matrix_left<f32, 8, 8>, 4>;
  var m_nested_array : array<array<subgroup_matrix_left<f32, 8, 8>, 4>, 4>;
  var m_struct : S;
  var m_nested_struct : S_Nested;
  let m_let = m;
  let m_array_let = m_array;
  let m_nested_array_let = m_nested_array;
  let m_struct_let = m_struct;
  let m_nested_struct_let = m_nested_struct;
  subgroupMatrixStore(&(buffer), 0, m_let, false, 64);
  subgroupMatrixStore(&(buffer), 0, m_array_let[0], false, 64);
  subgroupMatrixStore(&(buffer), 0, m_nested_array_let[1][2], false, 64);
  subgroupMatrixStore(&(buffer), 0, m_struct_let.l, false, 64);
  subgroupMatrixStore(&(buffer), 0, m_nested_struct_let.s.r, false, 64);
}
