SKIP: FAILED

binary/mul/mat3x2-vec3/f16.wgsl:3:14 error: using f16 types in 'uniform' storage class is not implemented yet
    matrix : mat3x2<f16>,
             ^^^^^^^^^^^

binary/mul/mat3x2-vec3/f16.wgsl:2:1 note: see layout of struct:
/*            align(8) size(24) */ struct S {
/* offset( 0) align(4) size(12) */   matrix : mat3x2<f16>;
/* offset(12) align(1) size( 4) */   // -- implicit field alignment padding --;
/* offset(16) align(8) size( 6) */   vector : vec3<f16>;
/* offset(22) align(1) size( 2) */   // -- implicit struct size padding --;
/*                              */ };
struct S {
^^^^^^

binary/mul/mat3x2-vec3/f16.wgsl:6:36 note: see declaration of variable
@group(0) @binding(0) var<uniform> data: S;
                                   ^^^^

