#version 310 es

shared uint wg[4];
void compute_main(uint local_invocation_index) {
  {
    for(uint idx = local_invocation_index; (idx < 4u); idx = (idx + 1u)) {
      uint i = idx;
      atomicExchange(wg[i], 0u);
    }
  }
  barrier();
  atomicExchange(wg[1], 1u);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
