struct S {
  field0 : f32;
  age : f32;
};

struct S_1 {
  field0 : f32;
  ancientness : f32;
};

var<private> myvar : S;

var<private> myvar2 : S_1;

fn main_1() {
  myvar.age = 42.0;
  myvar2.ancientness = 420.0;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
