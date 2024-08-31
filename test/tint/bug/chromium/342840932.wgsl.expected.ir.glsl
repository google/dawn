#version 310 es

uniform highp readonly uimage1D image_dup_src;
uniform highp writeonly uimage1D image_dst;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
