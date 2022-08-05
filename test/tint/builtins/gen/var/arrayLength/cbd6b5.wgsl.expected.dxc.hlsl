SKIP: FAILED

gen/var/arrayLength/cbd6b5.wgsl:26:10 error: using f16 types in 'storage' storage class is not implemented yet
  arg_0: array<f16>,
         ^^^^^^^^^^

gen/var/arrayLength/cbd6b5.wgsl:25:1 note: see layout of struct:
/*           align(2) size(2) */ struct SB_RW {
/* offset(0) align(2) size(2) */   arg_0 : array<f16>;
/*                            */ };
struct SB_RW {
^^^^^^

gen/var/arrayLength/cbd6b5.wgsl:28:48 note: see declaration of variable
@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;
                                               ^^^^^

