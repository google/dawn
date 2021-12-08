SKIP: FAILED


[[block]]
struct buf0 {
  resolution : vec2<f32>;
};

struct S {
  field0 : u32;
  field1 : u32;
};

[[group(0), binding(0)]] var<uniform> x_75 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

error: extended arithmetic is not finalized for WGSL: https://github.com/gpuweb/gpuweb/issues/1565: %712 = OpISubBorrow %710 %107 %470
