struct S {
  field0 : mat3x3<f32>,
}

struct S_1 {
  field0 : array<array<array<S, 83u>, 21u>, 47u>,
}

struct S_2 {
  field0 : array<array<vec3<f32>, 37u>, 95u>,
}

struct S_3 {
  field0 : S_2,
}

struct S_4 {
  field0 : array<vec2<i32>, 56u>,
}

struct S_5 {
  field0 : S_4,
}

struct S_6 {
  field0 : array<array<vec3<f32>, 18u>, 13u>,
}

struct S_7 {
  field0 : array<vec2<i32>, 88u>,
}

const x_72 = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);

const x_73 = mat4x4<f32>(x_72, x_72, x_72, x_72);

var<private> x_75 : array<mat4x4<f32>, 58u> = array<mat4x4<f32>, 58u>(x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73, x_73);

const x_77 = vec3<f32>(0.0f, 0.0f, 0.0f);

const x_78 = array<vec3<f32>, 18u>(x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77);

const x_80 = S_6(array<array<vec3<f32>, 18u>, 13u>(x_78, x_78, x_78, x_78, x_78, x_78, x_78, x_78, x_78, x_78, x_78, x_78, x_78));

var<private> x_82 : array<S_6, 46u> = array<S_6, 46u>(x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80, x_80);

var<private> x_85 : array<vec3<f32>, 37u> = array<vec3<f32>, 37u>(x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77, x_77);

fn main_1() {
  let x_88 : u32 = 58u;
  return;
}

@fragment
fn main() {
  main_1();
}
