SKIP: FAILED

binary/mul/vec3-mat4x3/f16.wgsl:3:14 error: using f16 types in 'uniform' storage class is not implemented yet
    matrix : mat4x3<f16>,
             ^^^^^^^^^^^

binary/mul/vec3-mat4x3/f16.wgsl:2:1 note: see layout of struct:
/*            align(8) size(40) */ struct S {
/* offset( 0) align(8) size(32) */   matrix : mat4x3<f16>;
/* offset(32) align(8) size( 6) */   vector : vec3<f16>;
/* offset(38) align(1) size( 2) */   // -- implicit struct size padding --;
/*                              */ };
struct S {
^^^^^^

binary/mul/vec3-mat4x3/f16.wgsl:6:36 note: see declaration of variable
@group(0) @binding(0) var<uniform> data: S;
                                   ^^^^

