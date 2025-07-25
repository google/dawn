enable chromium_experimental_subgroup_matrix;

struct S {
  l : subgroup_matrix_left<f32, 8, 8>,
  r : subgroup_matrix_right<f32, 8, 8>,
}

struct S_Nested {
  s : S,
}

@group(0) @binding(0) var<storage, read_write> buffer : array<f32>;

fn make_matrix() -> subgroup_matrix_left<f32, 8, 8> {
  var m : subgroup_matrix_left<f32, 8, 8>;
  return m;
}

fn make_array() -> array<subgroup_matrix_left<f32, 8, 8>, 4> {
  var m_array : array<subgroup_matrix_left<f32, 8, 8>, 4>;
  return m_array;
}

fn make_nested_array() -> array<array<subgroup_matrix_left<f32, 8, 8>, 4>, 4> {
  var m_nested_array : array<array<subgroup_matrix_left<f32, 8, 8>, 4>, 4>;
  return m_nested_array;
}

fn make_struct() -> S {
  var m_struct : S;
  return m_struct;
}

fn make_nested_struct() -> S_Nested {
  var m_nested_struct : S_Nested;
  return m_nested_struct;
}

@compute @workgroup_size(64)
fn main() {
  subgroupMatrixStore(&(buffer), 0, make_matrix(), false, 64);
  subgroupMatrixStore(&(buffer), 0, make_array()[0], false, 64);
  subgroupMatrixStore(&(buffer), 0, make_nested_array()[1][2], false, 64);
  subgroupMatrixStore(&(buffer), 0, make_struct().l, false, 64);
  subgroupMatrixStore(&(buffer), 0, make_nested_struct().s.r, false, 64);
}
