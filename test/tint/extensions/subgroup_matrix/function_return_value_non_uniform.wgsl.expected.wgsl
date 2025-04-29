enable chromium_experimental_subgroup_matrix;
diagnostic(off, chromium.subgroup_matrix_uniformity);

struct S {
  l : subgroup_matrix_left<f32, 8, 8>,
  r : subgroup_matrix_right<f32, 8, 8>,
}

struct S_Nested {
  s : S,
}

@group(0) @binding(0) var<storage, read_write> buffer : array<f32>;

var<private> non_uniform_condition : bool;

fn make_matrix() -> subgroup_matrix_left<f32, 8, 8> {
  if (non_uniform_condition) {
    return subgroup_matrix_left<f32, 8, 8>(1);
  } else {
    return subgroup_matrix_left<f32, 8, 8>(2);
  }
}

fn make_array() -> array<subgroup_matrix_left<f32, 8, 8>, 2> {
  if (non_uniform_condition) {
    return array<subgroup_matrix_left<f32, 8, 8>, 2>(subgroup_matrix_left<f32, 8, 8>(42), subgroup_matrix_left<f32, 8, 8>(100));
  } else {
    return array<subgroup_matrix_left<f32, 8, 8>, 2>(subgroup_matrix_left<f32, 8, 8>(-(7)), subgroup_matrix_left<f32, 8, 8>(-(42)));
  }
}

fn make_nested_array() -> array<array<subgroup_matrix_left<f32, 8, 8>, 2>, 2> {
  var a : array<array<subgroup_matrix_left<f32, 8, 8>, 2>, 2>;
  var b : array<array<subgroup_matrix_left<f32, 8, 8>, 2>, 2>;
  if (non_uniform_condition) {
    return array<array<subgroup_matrix_left<f32, 8, 8>, 2>, 2>(array<subgroup_matrix_left<f32, 8, 8>, 2>(subgroup_matrix_left<f32, 8, 8>(42), subgroup_matrix_left<f32, 8, 8>(100)), array<subgroup_matrix_left<f32, 8, 8>, 2>(subgroup_matrix_left<f32, 8, 8>(-(7)), subgroup_matrix_left<f32, 8, 8>(-(42))));
  } else {
    return array<array<subgroup_matrix_left<f32, 8, 8>, 2>, 2>(array<subgroup_matrix_left<f32, 8, 8>, 2>(subgroup_matrix_left<f32, 8, 8>(7), subgroup_matrix_left<f32, 8, 8>(42)), array<subgroup_matrix_left<f32, 8, 8>, 2>(subgroup_matrix_left<f32, 8, 8>(-(100)), subgroup_matrix_left<f32, 8, 8>(-(1))));
  }
}

fn make_struct() -> S {
  if (non_uniform_condition) {
    return S(subgroup_matrix_left<f32, 8, 8>(42), subgroup_matrix_right<f32, 8, 8>(100));
  } else {
    return S(subgroup_matrix_left<f32, 8, 8>(-(7)), subgroup_matrix_right<f32, 8, 8>(-(42)));
  }
}

fn make_nested_struct() -> S_Nested {
  if (non_uniform_condition) {
    return S_Nested(S(subgroup_matrix_left<f32, 8, 8>(42), subgroup_matrix_right<f32, 8, 8>(100)));
  } else {
    return S_Nested(S(subgroup_matrix_left<f32, 8, 8>(-(7)), subgroup_matrix_right<f32, 8, 8>(-(42))));
  }
}

@compute @workgroup_size(64)
fn main(@builtin(local_invocation_index) idx : u32) {
  non_uniform_condition = (buffer[idx] == 0);
  subgroupMatrixStore(&(buffer), 0, make_matrix(), false, 64);
  subgroupMatrixStore(&(buffer), 0, make_array()[0], false, 64);
  subgroupMatrixStore(&(buffer), 0, make_nested_array()[1][0], false, 64);
  subgroupMatrixStore(&(buffer), 0, make_struct().l, false, 64);
  subgroupMatrixStore(&(buffer), 0, make_nested_struct().s.r, false, 64);
}
