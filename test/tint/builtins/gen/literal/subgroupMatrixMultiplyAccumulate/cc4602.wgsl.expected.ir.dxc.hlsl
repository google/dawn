SKIP: FAILED

<dawn>/test/tint/builtins/gen/literal/subgroupMatrixMultiplyAccumulate/cc4602.wgsl:42:62 error: type 'i8' cannot be used in address space 'storage' as it is non-host-shareable
@group(0) @binding(0) var<storage, read_write> prevent_dce : array<i8, 1024>;
                                                             ^^^^^^^^^^^^^^^

<dawn>/test/tint/builtins/gen/literal/subgroupMatrixMultiplyAccumulate/cc4602.wgsl:42:23 note: while instantiating 'var' prevent_dce
@group(0) @binding(0) var<storage, read_write> prevent_dce : array<i8, 1024>;
                      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
