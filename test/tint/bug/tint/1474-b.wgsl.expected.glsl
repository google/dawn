<dawn>/test/tint/bug/tint/1474-b.wgsl:7:5 warning: code is unreachable
    let non_uniform_cond = non_uniform_value == 0;
    ^^^^^^^^^^^^^^^^^^^^

#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
