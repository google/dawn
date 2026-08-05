SKIP: FAILED

DXC validation failure:
hlsl.hlsl:29:27: error: no matching function for call to 'Load'
  Matrix_left_f32_8x8 m = Matrix_left_f32_8x8::Load(v, ((0u / 4u) + ((select(v_5, v_3, 0u) * 4u) / 4u)), select(v_5, v_4, 8u), MatrixLayout::ColMajor);
                          ^~~~~~~~~~~~~~~~~~~~~~~~~
<built-in:hlsl>/dx/linalg.h:263:31: note: candidate function [with Align = 128] not viable: no known conversion from 'uint __attribute__((address_space(3)))[256]' to 'ByteAddressBuffer' for 1st argument
  [[nodiscard]] static Matrix Load(ByteAddressBuffer Res, uint StartOffset,
                              ^
<built-in:hlsl>/dx/linalg.h:272:31: note: candidate function [with Align = 128] not viable: no known conversion from 'uint __attribute__((address_space(3)))[256]' to 'RWByteAddressBuffer' for 1st argument
  [[nodiscard]] static Matrix Load(RWByteAddressBuffer Res, uint StartOffset,
                              ^
<built-in:hlsl>/dx/linalg.h:282:7: note: candidate template ignored: disabled by 'enable_if' [with T = unsigned int, Size = 256]
      (hlsl::is_same<typename hlsl::strip_vector_type<T>::type,
      ^
hlsl.hlsl:33:5: error: no matching member function for call to 'Store'
  m.Store(v, ((0u / 4u) + ((select(v_8, v_6, 0u) * 4u) / 4u)), select(v_8, v_7, 8u), MatrixLayout::RowMajor);
  ~~^~~~~
<built-in:hlsl>/dx/linalg.h:326:8: note: candidate function [with Align = 128] not viable: no known conversion from 'uint __attribute__((address_space(3)))[256]' to 'RWByteAddressBuffer' for 1st argument
  void Store(RWByteAddressBuffer Res, uint StartOffset, uint Stride,
       ^
<built-in:hlsl>/dx/linalg.h:334:7: note: candidate template ignored: disabled by 'enable_if' [with T = unsigned int, Size = 256]
      (hlsl::is_same<typename hlsl::strip_vector_type<T>::type,
      ^

#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_f32_8x8 = Matrix<ComponentType::F32, 8, 8, MatrixUse::A, MatrixScope::Wave>;
struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared uint v[256];
void main_inner(uint tint_local_index) {
  {
    uint v_1 = 0u;
    v_1 = tint_local_index;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 256u)) {
        break;
      }
      v[((v_2 * 4u) / 4u)] = 0u;
      {
        v_1 = (v_2 + 32u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  uint v_3 = asuint(int(0));
  uint v_4 = asuint(int(8));
  bool v_5 = (((v_3 + (v_4 * 7u)) + 8u) <= ((1024u - 0u) / 4u));
  Matrix_left_f32_8x8 m = Matrix_left_f32_8x8::Load(v, ((0u / 4u) + ((select(v_5, v_3, 0u) * 4u) / 4u)), select(v_5, v_4, 8u), MatrixLayout::ColMajor);
  uint v_6 = asuint(int(0));
  uint v_7 = asuint(int(8));
  bool v_8 = (((v_6 + (v_7 * 7u)) + 8u) <= ((1024u - 0u) / 4u));
  m.Store(v, ((0u / 4u) + ((select(v_8, v_6, 0u) * 4u) / 4u)), select(v_8, v_7, 8u), MatrixLayout::RowMajor);
}

[numthreads(32, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}


tint executable returned error: exit status 1
