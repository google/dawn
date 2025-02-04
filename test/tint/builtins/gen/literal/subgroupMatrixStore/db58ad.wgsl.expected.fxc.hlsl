SKIP: INVALID


enable chromium_experimental_subgroup_matrix;

struct SB_RW {
  arg_0 : array<u32, 64>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn subgroupMatrixStore_db58ad() {
  subgroupMatrixStore(&(sb_rw.arg_0), 1u, subgroup_matrix_result<u32, 8, 8>(), true, 1u);
}

@compute @workgroup_size(1)
fn compute_main() {
  subgroupMatrixStore_db58ad();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/subgroupMatrixStore/db58ad.wgsl:48:3 error: no matching call to 'subgroupMatrixStore(array<u32, 64>, u32, subgroup_matrix_result<u32, 8, 8>, bool, u32)'

4 candidate functions:
 • 'subgroupMatrixStore(ptr<workgroup' or 'storage, array<u32, AC>, write' or 'read_write>  ✗ , u32  ✓ , subgroup_matrix<K, u32, C, R>  ✓ , bool  ✓ , u32  ✓ )'
 • 'subgroupMatrixStore(ptr<workgroup' or 'storage, array<f16, AC>, write' or 'read_write>  ✗ , u32  ✓ , subgroup_matrix<K, f16, C, R>  ✗ , bool  ✓ , u32  ✓ )'
 • 'subgroupMatrixStore(ptr<workgroup' or 'storage, array<f32, AC>, write' or 'read_write>  ✗ , u32  ✓ , subgroup_matrix<K, f32, C, R>  ✗ , bool  ✓ , u32  ✓ )'
 • 'subgroupMatrixStore(ptr<workgroup' or 'storage, array<i32, AC>, write' or 'read_write>  ✗ , u32  ✓ , subgroup_matrix<K, i32, C, R>  ✗ , bool  ✓ , u32  ✓ )'

  subgroupMatrixStore(&sb_rw.arg_0, 1u, subgroup_matrix_result<u32, 8, 8>(), true, 1u);
  ^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
