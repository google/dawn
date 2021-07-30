fn first_shader_1() {
  return;
}

[[stage(fragment)]]
fn first_shader() {
  first_shader_1();
}

[[stage(fragment)]]
fn second_shader() {
  first_shader_1();
}
