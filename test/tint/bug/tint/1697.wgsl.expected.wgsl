SKIP: FAILED

<dawn>/test/tint/bug/tint/1697.wgsl:5:11 error: missing initializer for 'var' declaration
  var v = __tint_materialize(vec2(0))[i32(((__tint_materialize(vec2(smaller_than_any_f32))[O] * 1000000000000000013287555072.0) * 1000000000000000013287555072.0))];
          ^

const O = 0;

fn f() {
  const smaller_than_any_f32 = 1e-50;
  var v = __tint_materialize(vec2(0))[i32(((__tint_materialize(vec2(smaller_than_any_f32))[O] * 1000000000000000013287555072.0) * 1000000000000000013287555072.0))];
}
