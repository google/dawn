SKIP: FAILED

DXC validation failure:
hlsl.hlsl:75:29: error: no matching function for call to 'Load'
  Matrix_right_u32_8x8 m5 = Matrix_right_u32_8x8::Load(in5, ((0u / 2u) + ((asuint(int(0)) * 4u) / 2u)), ((16u * 4u) / 2u), MatrixLayout::ColMajor);
                            ^~~~~~~~~~~~~~~~~~~~~~~~~~
<built-in:hlsl>/dx/linalg.h:263:31: note: candidate function [with Align = 128] not viable: no known conversion from 'uint16_t __attribute__((address_space(3)))[2048]' to 'ByteAddressBuffer' for 1st argument
  [[nodiscard]] static Matrix Load(ByteAddressBuffer Res, uint StartOffset,
                              ^
<built-in:hlsl>/dx/linalg.h:272:31: note: candidate function [with Align = 128] not viable: no known conversion from 'uint16_t __attribute__((address_space(3)))[2048]' to 'RWByteAddressBuffer' for 1st argument
  [[nodiscard]] static Matrix Load(RWByteAddressBuffer Res, uint StartOffset,
                              ^
<built-in:hlsl>/dx/linalg.h:282:7: note: candidate template ignored: disabled by 'enable_if' [with T = uint16_t, Size = 2048]
      (hlsl::is_same<typename hlsl::strip_vector_type<T>::type,
      ^
hlsl.hlsl:82:29: error: no matching function for call to 'Load'
  Matrix_right_u32_8x8 m6 = Matrix_right_u32_8x8::Load(in6, ((0u / 2u) + ((asuint(int(0)) * 8u) / 2u)), ((16u * 8u) / 2u), MatrixLayout::ColMajor);
                            ^~~~~~~~~~~~~~~~~~~~~~~~~~
<built-in:hlsl>/dx/linalg.h:263:31: note: candidate function [with Align = 128] not viable: no known conversion from 'uint16_t __attribute__((address_space(3)))[4096]' to 'ByteAddressBuffer' for 1st argument
  [[nodiscard]] static Matrix Load(ByteAddressBuffer Res, uint StartOffset,
                              ^
<built-in:hlsl>/dx/linalg.h:272:31: note: candidate function [with Align = 128] not viable: no known conversion from 'uint16_t __attribute__((address_space(3)))[4096]' to 'RWByteAddressBuffer' for 1st argument
  [[nodiscard]] static Matrix Load(RWByteAddressBuffer Res, uint StartOffset,
                              ^
<built-in:hlsl>/dx/linalg.h:282:7: note: candidate template ignored: disabled by 'enable_if' [with T = uint16_t, Size = 4096]
      (hlsl::is_same<typename hlsl::strip_vector_type<T>::type,
      ^
<built-in:hlsl>/dx/linalg.h:290:60: error: cannot initialize a parameter of type 'unsigned int const __attribute__((address_space(3))) (&)[1024]' with an lvalue of type 'vector<unsigned int, 2> __attribute__((address_space(3)))[1024]'
    __builtin_LinAlg_MatrixLoadFromMemory(Result.__handle, Arr, StartIdx,
                                                           ^~~
hlsl.hlsl:54:51: note: in instantiation of function template specialization 'dx::linalg::Matrix<dx::linalg::ComponentType::ComponentEnum::U32, 8, 8, dx::linalg::MatrixUse::MatrixUseEnum::B, dx::linalg::MatrixScope::MatrixScopeEnum::Wave>::Load<vector<unsigned int, 2>, 1024>' requested here
  Matrix_right_u32_8x8 m1 = Matrix_right_u32_8x8::Load(in1, asuint(int(0)), 16u, MatrixLayout::ColMajor);
                                                  ^
In file included from hlsl.hlsl:1:
<built-in:hlsl>/dx/linalg.h:290:60: error: cannot initialize a parameter of type 'unsigned int const __attribute__((address_space(3))) (&)[1024]' with an lvalue of type 'vector<unsigned int, 4> __attribute__((address_space(3)))[1024]'
    __builtin_LinAlg_MatrixLoadFromMemory(Result.__handle, Arr, StartIdx,
                                                           ^~~
hlsl.hlsl:68:51: note: in instantiation of function template specialization 'dx::linalg::Matrix<dx::linalg::ComponentType::ComponentEnum::U32, 8, 8, dx::linalg::MatrixUse::MatrixUseEnum::B, dx::linalg::MatrixScope::MatrixScopeEnum::Wave>::Load<vector<unsigned int, 4>, 1024>' requested here
  Matrix_right_u32_8x8 m3 = Matrix_right_u32_8x8::Load(in3, asuint(int(0)), 16u, MatrixLayout::ColMajor);
                                                  ^

#include <dx/linalg.h>
using namespace dx::linalg;
using Matrix_right_u32_8x8 = Matrix<ComponentType::U32, 8, 8, MatrixUse::B, MatrixScope::Wave>;
struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared uint in0[1024];
groupshared uint2 in1[1024];
groupshared uint in2[4096];
groupshared uint4 in3[1024];
groupshared uint16_t in5[2048];
groupshared uint16_t in6[4096];
RWByteAddressBuffer v : register(u0);
void main_inner(uint tint_local_index) {
  {
    uint v_1 = 0u;
    v_1 = tint_local_index;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 1024u)) {
        break;
      }
      in0[v_2] = 0u;
      in1[((v_2 * 8u) / 8u)] = asuint(int2((int(0)).xx));
      uint v_3 = ((v_2 * 16u) / 4u);
      in2[v_3] = asuint((0.0f).xxx.x);
      uint v_4 = (v_3 + 1u);
      in2[v_4] = asuint((0.0f).xxx.y);
      in2[(v_4 + 1u)] = asuint((0.0f).xxx.z);
      in3[v_2] = (0u).xxxx;
      uint v_5 = ((v_2 * 4u) / 2u);
      in5[v_5] = asuint16((float16_t(0.0h)).xx.x);
      in5[(v_5 + 1u)] = asuint16((float16_t(0.0h)).xx.y);
      uint v_6 = ((v_2 * 8u) / 2u);
      in6[v_6] = asuint16((float16_t(0.0h)).xxx.x);
      uint v_7 = (v_6 + 1u);
      in6[v_7] = asuint16((float16_t(0.0h)).xxx.y);
      in6[(v_7 + 1u)] = asuint16((float16_t(0.0h)).xxx.z);
      {
        v_1 = (v_2 + 64u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  Matrix_right_u32_8x8 m0 = Matrix_right_u32_8x8::Load(in0, asuint(int(0)), 16u, MatrixLayout::ColMajor);
  uint v_8 = 0u;
  v.GetDimensions(v_8);
  uint v_9 = asuint(int(0));
  uint v_10 = asuint(int(16));
  bool v_11 = (((v_9 + (v_10 * 7u)) + 8u) <= (v_8 / 4u));
  m0.Store(v, (0u + (select(v_11, v_9, 0u) * 4u)), (select(v_11, v_10, 8u) * 4u), MatrixLayout::ColMajor);
  Matrix_right_u32_8x8 m1 = Matrix_right_u32_8x8::Load(in1, asuint(int(0)), 16u, MatrixLayout::ColMajor);
  uint v_12 = 0u;
  v.GetDimensions(v_12);
  uint v_13 = asuint(int(0));
  uint v_14 = asuint(int(16));
  bool v_15 = (((v_13 + (v_14 * 7u)) + 8u) <= (v_12 / 4u));
  m1.Store(v, (0u + (select(v_15, v_13, 0u) * 4u)), (select(v_15, v_14, 8u) * 4u), MatrixLayout::ColMajor);
  Matrix_right_u32_8x8 m2 = Matrix_right_u32_8x8::Load(in2, ((0u / 4u) + ((asuint(int(0)) * 16u) / 4u)), ((16u * 16u) / 4u), MatrixLayout::ColMajor);
  uint v_16 = 0u;
  v.GetDimensions(v_16);
  uint v_17 = asuint(int(0));
  uint v_18 = asuint(int(16));
  bool v_19 = (((v_17 + (v_18 * 7u)) + 8u) <= (v_16 / 4u));
  m2.Store(v, (0u + (select(v_19, v_17, 0u) * 4u)), (select(v_19, v_18, 8u) * 4u), MatrixLayout::ColMajor);
  Matrix_right_u32_8x8 m3 = Matrix_right_u32_8x8::Load(in3, asuint(int(0)), 16u, MatrixLayout::ColMajor);
  uint v_20 = 0u;
  v.GetDimensions(v_20);
  uint v_21 = asuint(int(0));
  uint v_22 = asuint(int(16));
  bool v_23 = (((v_21 + (v_22 * 7u)) + 8u) <= (v_20 / 4u));
  m3.Store(v, (0u + (select(v_23, v_21, 0u) * 4u)), (select(v_23, v_22, 8u) * 4u), MatrixLayout::ColMajor);
  Matrix_right_u32_8x8 m5 = Matrix_right_u32_8x8::Load(in5, ((0u / 2u) + ((asuint(int(0)) * 4u) / 2u)), ((16u * 4u) / 2u), MatrixLayout::ColMajor);
  uint v_24 = 0u;
  v.GetDimensions(v_24);
  uint v_25 = asuint(int(0));
  uint v_26 = asuint(int(16));
  bool v_27 = (((v_25 + (v_26 * 7u)) + 8u) <= (v_24 / 4u));
  m5.Store(v, (0u + (select(v_27, v_25, 0u) * 4u)), (select(v_27, v_26, 8u) * 4u), MatrixLayout::ColMajor);
  Matrix_right_u32_8x8 m6 = Matrix_right_u32_8x8::Load(in6, ((0u / 2u) + ((asuint(int(0)) * 8u) / 2u)), ((16u * 8u) / 2u), MatrixLayout::ColMajor);
  uint v_28 = 0u;
  v.GetDimensions(v_28);
  uint v_29 = asuint(int(0));
  uint v_30 = asuint(int(16));
  bool v_31 = (((v_29 + (v_30 * 7u)) + 8u) <= (v_28 / 4u));
  m6.Store(v, (0u + (select(v_31, v_29, 0u) * 4u)), (select(v_31, v_30, 8u) * 4u), MatrixLayout::ColMajor);
}

[numthreads(64, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}


tint executable returned error: exit status 1
