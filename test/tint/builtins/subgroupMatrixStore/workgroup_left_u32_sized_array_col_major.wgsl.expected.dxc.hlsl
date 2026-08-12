SKIP: FAILED

DXC validation failure:
hlsl.hlsl:51:5: error: no matching member function for call to 'Store'
  m.Store(out5, ((0u / 2u) + ((asuint(int(0)) * 4u) / 2u)), ((16u * 4u) / 2u), MatrixLayout::ColMajor);
  ~~^~~~~
<built-in:hlsl>/dx/linalg.h:326:8: note: candidate function [with Align = 128] not viable: no known conversion from 'uint16_t __attribute__((address_space(3)))[2048]' to 'RWByteAddressBuffer' for 1st argument
  void Store(RWByteAddressBuffer Res, uint StartOffset, uint Stride,
       ^
<built-in:hlsl>/dx/linalg.h:334:7: note: candidate template ignored: disabled by 'enable_if' [with T = uint16_t, Size = 2048]
      (hlsl::is_same<typename hlsl::strip_vector_type<T>::type,
      ^
hlsl.hlsl:52:5: error: no matching member function for call to 'Store'
  m.Store(out6, ((0u / 2u) + ((asuint(int(0)) * 8u) / 2u)), ((16u * 8u) / 2u), MatrixLayout::ColMajor);
  ~~^~~~~
<built-in:hlsl>/dx/linalg.h:326:8: note: candidate function [with Align = 128] not viable: no known conversion from 'uint16_t __attribute__((address_space(3)))[4096]' to 'RWByteAddressBuffer' for 1st argument
  void Store(RWByteAddressBuffer Res, uint StartOffset, uint Stride,
       ^
<built-in:hlsl>/dx/linalg.h:334:7: note: candidate template ignored: disabled by 'enable_if' [with T = uint16_t, Size = 4096]
      (hlsl::is_same<typename hlsl::strip_vector_type<T>::type,
      ^
<built-in:hlsl>/dx/linalg.h:341:52: error: cannot initialize a parameter of type 'unsigned int const __attribute__((address_space(3))) (&)[1024]' with an lvalue of type 'vector<unsigned int, 2> __attribute__((address_space(3)))[1024]'
    __builtin_LinAlg_MatrixStoreToMemory(__handle, Arr, StartIdx, Stride,
                                                   ^~~
hlsl.hlsl:48:5: note: in instantiation of function template specialization 'dx::linalg::Matrix<dx::linalg::ComponentType::ComponentEnum::U32, 8, 8, dx::linalg::MatrixUse::MatrixUseEnum::A, dx::linalg::MatrixScope::MatrixScopeEnum::Wave>::Store<vector<unsigned int, 2>, 1024>' requested here
  m.Store(out1, asuint(int(0)), 16u, MatrixLayout::ColMajor);
    ^
In file included from hlsl.hlsl:1:
<built-in:hlsl>/dx/linalg.h:341:52: error: cannot initialize a parameter of type 'unsigned int const __attribute__((address_space(3))) (&)[1024]' with an lvalue of type 'vector<unsigned int, 4> __attribute__((address_space(3)))[1024]'
    __builtin_LinAlg_MatrixStoreToMemory(__handle, Arr, StartIdx, Stride,
                                                   ^~~
hlsl.hlsl:50:5: note: in instantiation of function template specialization 'dx::linalg::Matrix<dx::linalg::ComponentType::ComponentEnum::U32, 8, 8, dx::linalg::MatrixUse::MatrixUseEnum::A, dx::linalg::MatrixScope::MatrixScopeEnum::Wave>::Store<vector<unsigned int, 4>, 1024>' requested here
  m.Store(out3, asuint(int(0)), 16u, MatrixLayout::ColMajor);
    ^

#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_left_u32_8x8 = Matrix<ComponentType::U32, 8, 8, MatrixUse::A, MatrixScope::Wave>;
struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared uint out0[1024];
groupshared uint2 out1[1024];
groupshared uint out2[4096];
groupshared uint4 out3[1024];
groupshared uint16_t out5[2048];
groupshared uint16_t out6[4096];
void main_inner(uint tint_local_index) {
  {
    uint v = 0u;
    v = tint_local_index;
    while(true) {
      uint v_1 = v;
      if ((v_1 >= 1024u)) {
        break;
      }
      out0[v_1] = 0u;
      out1[((v_1 * 8u) / 8u)] = asuint(int2((int(0)).xx));
      uint v_2 = ((v_1 * 16u) / 4u);
      out2[v_2] = asuint((0.0f).xxx.x);
      uint v_3 = (v_2 + 1u);
      out2[v_3] = asuint((0.0f).xxx.y);
      out2[(v_3 + 1u)] = asuint((0.0f).xxx.z);
      out3[v_1] = (0u).xxxx;
      uint v_4 = ((v_1 * 4u) / 2u);
      out5[v_4] = asuint16((float16_t(0.0h)).xx.x);
      out5[(v_4 + 1u)] = asuint16((float16_t(0.0h)).xx.y);
      uint v_5 = ((v_1 * 8u) / 2u);
      out6[v_5] = asuint16((float16_t(0.0h)).xxx.x);
      uint v_6 = (v_5 + 1u);
      out6[v_6] = asuint16((float16_t(0.0h)).xxx.y);
      out6[(v_6 + 1u)] = asuint16((float16_t(0.0h)).xxx.z);
      {
        v = (v_1 + 64u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  Matrix_left_u32_8x8 m = Matrix_left_u32_8x8::Splat(0u);
  m.Store(out0, asuint(int(0)), 16u, MatrixLayout::ColMajor);
  m.Store(out1, asuint(int(0)), 16u, MatrixLayout::ColMajor);
  m.Store(out2, ((0u / 4u) + ((asuint(int(0)) * 16u) / 4u)), ((16u * 16u) / 4u), MatrixLayout::ColMajor);
  m.Store(out3, asuint(int(0)), 16u, MatrixLayout::ColMajor);
  m.Store(out5, ((0u / 2u) + ((asuint(int(0)) * 4u) / 2u)), ((16u * 4u) / 2u), MatrixLayout::ColMajor);
  m.Store(out6, ((0u / 2u) + ((asuint(int(0)) * 8u) / 2u)), ((16u * 8u) / 2u), MatrixLayout::ColMajor);
}

[numthreads(64, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}


tint executable returned error: exit status 1
