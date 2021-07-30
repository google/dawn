void first_shader_1() {
  return;
}

void first_shader() {
  first_shader_1();
  return;
}

void second_shader() {
  first_shader_1();
  return;
}
