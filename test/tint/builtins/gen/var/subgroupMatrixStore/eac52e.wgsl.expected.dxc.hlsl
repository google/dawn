SKIP: FAILED


enable chromium_experimental_subgroup_matrix;

struct SB_RW {
  arg_0 : array<f32, 1024>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn subgroupMatrixStore_eac52e() {
  var arg_1 = 1u;
  var arg_2 = subgroup_matrix_left<f32, 8, 8>();
  const arg_3 = true;
  var arg_4 = 8u;
  subgroupMatrixStore(&(sb_rw.arg_0), arg_1, arg_2, arg_3, arg_4);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_eac52e();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupMatrixStore/eac52e.wgsl:53:3 error: no matching call to 'subgroupMatrixStore(array<f32, 1024>, u32, subgroup_matrix_left<f32, 8, 8>, bool, u32)'

6 candidate functions:
 • 'subgroupMatrixStore(ptr<storage, array<S>, write' or 'read_write>  ✗ , u32  ✓ , subgroup_matrix<K, S, C, R>  ✓ , bool  ✓ , u32  ✓ )' where:
      ✓  'S' is 'f32', 'i32', 'u32' or 'f16'
 • 'subgroupMatrixStore(ptr<workgroup' or 'storage, array<S, AC>, write' or 'read_write>  ✗ , u32  ✓ , subgroup_matrix<K, S, C, R>  ✓ , bool  ✓ , u32  ✓ )' where:
      ✓  'S' is 'f32', 'i32', 'u32' or 'f16'
 • 'subgroupMatrixStore(ptr<storage, array<i32>, write' or 'read_write>  ✗ , u32  ✓ , subgroup_matrix<K, S, C, R>  ✗ , bool  ✓ , u32  ✓ )' where:
      ✗  'S' is 'i8'
 • 'subgroupMatrixStore(ptr<storage, array<u32>, write' or 'read_write>  ✗ , u32  ✓ , subgroup_matrix<K, S, C, R>  ✗ , bool  ✓ , u32  ✓ )' where:
      ✗  'S' is 'u8'
 • 'subgroupMatrixStore(ptr<workgroup' or 'storage, array<i32, AC>, write' or 'read_write>  ✗ , u32  ✓ , subgroup_matrix<K, S, C, R>  ✗ , bool  ✓ , u32  ✓ )' where:
      ✗  'S' is 'i8'
 • 'subgroupMatrixStore(ptr<workgroup' or 'storage, array<u32, AC>, write' or 'read_write>  ✗ , u32  ✓ , subgroup_matrix<K, S, C, R>  ✗ , bool  ✓ , u32  ✓ )' where:
      ✗  'S' is 'u8'

  subgroupMatrixStore(&sb_rw.arg_0, arg_1, arg_2, arg_3, arg_4);
  ^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
