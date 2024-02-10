# vkperf sample output on Nvidia Quadro RTX 3000

```
vkperf (0.99.5) tests various performance characteristics of Vulkan devices.

Devices in the system:
   Quadro RTX 3000
   Intel(R) UHD Graphics

Selected device:
   Quadro RTX 3000

VendorID:  0x10de (Nvidia)
DeviceID:  0x1f36
Vulkan version:  1.3.242
Driver version:  536.45.0.0 (2248884224, 0x860b4000)
   Driver name:  NVIDIA
   Driver info:  536.45
   DriverID:     NvidiaProprietary
   Driver conformance version:  1.3.5.0
GPU memory:  6GiB  (5968MiB + 214MiB)
Max memory allocations:  4096
Standard (non-sparse) buffer alignment:  16
Number of triangles for tests:  1000000
Sparse mode for tests:  None
Timestamp number of bits:  64
Timestamp period:  1ns
Vulkan Instance version:  1.3.241
Operating system:  Windows 10 Pro
                   Version number: 10.0, build number: 19045
Processor:  Intel(R) Core(TM) i7-10875H CPU @ 2.30GHz

Triangle throughput:
   Triangle list (triangle list primitive type,
      single per-scene vkCmdDraw() call, attributeless,
      constant VS output):                     4.002 giga-triangles/s
   Indexed triangle list (triangle list primitive type, single
      per-scene vkCmdDrawIndexed() call, no vertices shared between triangles,
      attributeless, constant VS output):      4.095 giga-triangles/s
   Indexed triangle list that reuses two indices of the previous triangle
      (triangle list primitive type, single per-scene vkCmdDrawIndexed() call,
      attributeless, constant VS output):      13.05 giga-triangles/s
   Triangle strips of various lengths
      (per-strip vkCmdDraw() call, 1 to 1000 triangles per strip,
      attributeless, constant VS output):
         strip length 1:    286.3 mega-triangles/s
         strip length 2:    559.3 mega-triangles/s
         strip length 5:    1.393 giga-triangles/s
         strip length 8:    2.229 giga-triangles/s
         strip length 10:   2.821 giga-triangles/s
         strip length 20:   5.812 giga-triangles/s
         strip length 25:   7.436 giga-triangles/s
         strip length 40:   11.51 giga-triangles/s
         strip length 50:   11.99 giga-triangles/s
         strip length 100:  12.68 giga-triangles/s
         strip length 125:  11.62 giga-triangles/s
         strip length 1000: 11.10 giga-triangles/s
   Indexed triangle strips of various lengths
      (per-strip vkCmdDrawIndexed() call, 1-1000 triangles per strip,
      no vertices shared between strips, each index used just once,
      attributeless, constant VS output):
         strip length 1:    254.0 mega-triangles/s
         strip length 2:    522.6 mega-triangles/s
         strip length 5:    1.284 giga-triangles/s
         strip length 8:    2.068 giga-triangles/s
         strip length 10:   2.550 giga-triangles/s
         strip length 20:   5.307 giga-triangles/s
         strip length 25:   6.687 giga-triangles/s
         strip length 40:   10.65 giga-triangles/s
         strip length 50:   12.20 giga-triangles/s
         strip length 100:  11.95 giga-triangles/s
         strip length 125:  12.06 giga-triangles/s
         strip length 1000: 11.90 giga-triangles/s
   Primitive restart indexed triangle strips of various lengths
      (single per-scene vkCmdDrawIndexed() call, 1-1000 triangles per strip,
      no vertices shared between strips, each index used just once,
      attributeless, constant VS output):
         strip length 1:    1.521 giga-triangles/s
         strip length 2:    3.048 giga-triangles/s
         strip length 5:    6.802 giga-triangles/s
         strip length 8:    11.07 giga-triangles/s
         strip length 1000: 12.58 giga-triangles/s
   Primitive restart, each triangle is replaced by one -1
      (single per-scene vkCmdDrawIndexed() call,
      no fragments produced):                  1.648 giga-triangles/s
   Primitive restart, only zeros in the index buffer
      (single per-scene vkCmdDrawIndexed() call,
      no fragments produced):                  14.94 giga-triangles/s
   Instancing throughput of vkCmdDraw()
      (one triangle per instance, constant VS output, one draw call,
      attributeless):                          1.713 giga-triangles/s
   Instancing throughput of vkCmdDrawIndexed()
      (one triangle per instance, constant VS output, one draw call,
      attributeless):                          1.627 giga-triangles/s
   Instancing throughput of vkCmdDrawIndirect()
      (one triangle per instance, one indirect draw call,
      one indirect record, attributeless:      1.708 giga-triangles/s
   Instancing throughput of vkCmdDrawIndexedIndirect()
      (one triangle per instance, one indirect draw call,
      one indirect record, attributeless:      1.628 giga-triangles/s
   vkCmdDraw() throughput
      (per-triangle vkCmdDraw() in command buffer,
      attributeless, constant VS output):      274.6 mega-triangles/s
   vkCmdDrawIndexed() throughput
      (per-triangle vkCmdDrawIndexed() in command buffer,
      attributeless, constant VS output):      252.2 mega-triangles/s
   VkDrawIndirectCommand processing throughput
      (per-triangle VkDrawIndirectCommand, one vkCmdDrawIndirect() call,
      attributeless):                          186.8 mega-triangles/s
   VkDrawIndirectCommand processing throughput with stride 32
      (per-triangle VkDrawIndirectCommand, one vkCmdDrawIndirect() call,
      attributeless):                          132.8 mega-triangles/s
   VkDrawIndexedIndirectCommand processing throughput
      (per-triangle VkDrawIndexedIndirectCommand, 1x vkCmdDrawIndexedIndirect()
      call, attributeless):                    163.8 mega-triangles/s
   VkDrawIndexedIndirectCommand processing throughput with stride 32
      (per-triangle VkDrawIndexedIndirectCommand, 1x vkCmdDrawIndexedIndirect()
      call, attributeless):                    125.9 mega-triangles/s

Vertex and geometry shader throughput:
   VS throughput using vkCmdDraw() - minimal VS that just writes
      constant output position (per-scene vkCmdDraw() call,
      no attributes, no fragments produced):   13.07 giga-vertices/s
   VS throughput using vkCmdDrawIndexed() - minimal VS that just writes
      constant output position (per-scene vkCmdDrawIndexed() call,
      no attributes, no fragments produced):   13.16 giga-vertices/s
   VS producing output position from VertexIndex and InstanceIndex
      using vkCmdDraw() (single per-scene vkCmdDraw() call,
      attributeless, no fragments produced):   12.12 giga-vertices/s
   VS producing output position from VertexIndex and InstanceIndex
      using vkCmdDrawIndexed() (single per-scene vkCmdDrawIndexed() call,
      attributeless, no fragments produced):   12.05 giga-vertices/s
   GS one triangle in and no triangle out
      (empty VS, attributeless):               2.741 giga-invocations/s
   GS one triangle in and single constant triangle out
      (empty VS, attributeless):               2.730 giga-invocations/s
   GS one triangle in and two constant triangles out
      (empty VS, attributeless):               1.948 giga-invocations/s

Attributes and buffers:
   One attribute performance - 1x vec4 attribute
      (attribute used, per-scene draw call):   13.01 giga-vertices/s
   One buffer performance - 1x vec4 buffer
      (1x read in VS, per-scene draw call):    12.34 giga-vertices/s
   One buffer performance - 1x vec3 buffer
      (1x read in VS, one draw call):          12.30 giga-vertices/s
   Two attributes performance - 2x vec4 attribute
      (both attributes used):                  10.24 giga-vertices/s
   Two buffers performance - 2x vec4 buffer
      (both buffers read in VS):               10.33 giga-vertices/s
   Two buffers performance - 2x vec3 buffer
      (both buffers read in VS):               12.20 giga-vertices/s
   Two interleaved attributes performance - 2x vec4
      (2x vec4 attribute fetched from the single buffer in VS
      from consecutive buffer locations:       10.31 giga-vertices/s
   Two interleaved buffers performance - 2x vec4
      (2x vec4 fetched from the single buffer in VS
      from consecutive buffer locations:       10.74 giga-vertices/s
   Packed buffer performance - 1x buffer using 32-byte struct unpacked
      into position+normal+color+texCoord:     10.46 giga-vertices/s
   Packed attribute performance - 2x uvec4 attribute unpacked
      into position+normal+color+texCoord:     10.10 giga-vertices/s
   Packed buffer performance - 2x uvec4 buffers unpacked
      into position+normal+color+texCoord:     10.47 giga-vertices/s
   Packed buffer performance - 2x buffer using 16-byte struct unpacked
      into position+normal+color+texCoord:     10.40 giga-vertices/s
   Packed buffer performance - 2x buffer using 16-byte struct
      read multiple times and unpacked
      into position+normal+color+texCoord:     10.42 giga-vertices/s
   Four attributes performance - 4x vec4 attribute
      (all attributes used):                   5.142 giga-vertices/s
   Four buffers performance - 4x vec4 buffer
      (all buffers read in VS):                5.184 giga-vertices/s
   Four buffers performance - 4x vec3 buffer
      (all buffers read in VS):                6.920 giga-vertices/s
   Four interleaved attributes performance - 4x vec4
      (4x vec4 fetched from the single buffer
      on consecutive locations:                5.162 giga-vertices/s
   Four interleaved buffers performance - 4x vec4
      (4x vec4 fetched from the single buffer
      on consecutive locations:                5.193 giga-vertices/s
   Four attributes performance - 2x vec4 and 2x uint attribute
      (2x vec4f32 + 2x vec4u8, 2x conversion from vec4u8
      to vec4):                                6.208 giga-vertices/s

Transformations:
   Matrix performance - one matrix as uniform for all triangles
      (maxtrix read in VS,
      coordinates in vec4 attribute):          13.31 giga-vertices/s
   Matrix performance - per-triangle matrix in buffer
      (different matrix read for each triangle in VS,
      coordinates in vec4 attribute):          8.892 giga-vertices/s
   Matrix performance - per-triangle matrix in attribute
      (triangles are instanced and each triangle receives a different matrix,
      coordinates in vec4 attribute:           4.957 giga-vertices/s
   Matrix performance - one matrix in buffer for all triangles and 2x uvec4
      packed attributes (each triangle reads matrix from the same place in
      the buffer, attributes unpacked):        10.39 giga-vertices/s
   Matrix performance - per-triangle matrix in the buffer and 2x uvec4 packed
      attributes (each triangle reads a different matrix from a buffer,
      attributes unpacked):                    6.230 giga-vertices/s
   Matrix performance - per-triangle matrix in buffer and 2x uvec4 packed
      buffers (each triangle reads a different matrix from a buffer,
      packed buffers unpacked):                6.258 giga-vertices/s
   Matrix performance - GS reads per-triangle matrix from buffer and 2x uvec4
      packed buffers (each triangle reads a different matrix from a buffer,
      packed buffers unpacked in GS):          4.003 giga-vertices/s
   Matrix performance - per-triangle matrix in buffer and four attributes
      (each triangle reads a different matrix from a buffer,
      4x vec4 attribute):                      3.856 giga-vertices/s
   Matrix performance - 1x per-triangle matrix in buffer, 2x uniform matrix and
      and 2x uvec4 packed attributes (uniform view and projection matrices
      multiplied with per-triangle model matrix and with unpacked attributes of
      position, normal, color and texCoord:    6.228 giga-vertices/s
   Matrix performance - 2x per-triangle matrix (mat4+mat3) in buffer,
      3x uniform matrix (mat4+mat4+mat3) and 2x uvec4 packed attributes
      (full position and normal computation with MVP and normal matrices,
      all matrices and attributes multiplied): 4.790 giga-vertices/s
   Matrix performance - 2x per-triangle matrix (mat4+mat3) in buffer,
      2x non-changing matrix (mat4+mat4) in push constants,
      1x constant matrix (mat3) and 2x uvec4 packed attributes (all
      matrices and attributes multiplied):     4.771 giga-vertices/s
   Matrix performance - 2x per-triangle matrix (mat4+mat3) in buffer, 2x
      non-changing matrix (mat4+mat4) in specialization constants, 1x constant
      matrix (mat3) defined by VS code and 2x uvec4 packed attributes (all
      matrices and attributes multiplied):     4.794 giga-vertices/s
   Matrix performance - 2x per-triangle matrix (mat4+mat3) in buffer,
      3x constant matrix (mat4+mat4+mat3) defined by VS code and
      2x uvec4 packed attributes (all matrices and attributes
      multiplied):                             4.788 giga-vertices/s
   Matrix performance - GS five matrices processing, 2x per-triangle matrix
      (mat4+mat3) in buffer, 3x uniform matrix (mat4+mat4+mat3) and
      2x uvec4 packed attributes passed through VS (all matrices and
      attributes multiplied):                  3.937 giga-vertices/s
   Matrix performance - GS five matrices processing, 2x per-triangle matrix
      (mat4+mat3) in buffer, 3x uniform matrix (mat4+mat4+mat3) and
      2x uvec4 packed data read from buffer in GS (all matrices and attributes
      multiplied):                             3.197 giga-vertices/s
   Textured Phong and Matrix performance - 2x per-triangle matrix
      in buffer (mat4+mat3), 3x uniform matrix (mat4+mat4+mat3) and
      four attributes (vec4f32+vec3f32+vec4u8+vec2f32),
      no fragments produced:                   4.283 giga-vertices/s
   Textured Phong and Matrix performance - 1x per-triangle matrix
      in buffer (mat4), 2x uniform matrix (mat4+mat4) and
      four attributes (vec4f32+vec3f32+vec4u8+vec2f32),
      no fragments produced:                   5.348 giga-vertices/s
   Textured Phong and Matrix performance - 1x per-triangle matrix
      in buffer (mat4), 2x uniform matrix (mat4+mat4) and 2x uvec4 packed
      attribute, no fragments produced:        6.197 giga-vertices/s
   Textured Phong and Matrix performance - 1x per-triangle row-major matrix
      in buffer (mat4), 2x uniform not-row-major matrix (mat4+mat4),
      2x uvec4 packed attributes,
      no fragments produced:                   6.134 giga-vertices/s
   Textured Phong and Matrix performance - 1x per-triangle mat4x3 matrix
      in buffer, 2x uniform matrix (mat4+mat4) and 2x uvec4 packed attributes,
      no fragments produced:                   6.877 giga-vertices/s
   Textured Phong and Matrix performance - 1x per-triangle row-major mat4x3
      matrix in buffer, 2x uniform matrix (mat4+mat4), 2x uvec4 packed
      attribute, no fragments produced:        6.851 giga-vertices/s
   Textured Phong and PAT performance - PAT v1 (Position-Attitude-Transform,
      performing translation (vec3) and rotation (quaternion as vec4) using
      implementation 1), PAT is per-triangle 2x vec4 in buffer,
      2x uniform matrix (mat4+mat4), 2x uvec4 packed attributes,
      no fragments produced:                   7.716 giga-vertices/s
   Textured Phong and PAT performance - PAT v2 (Position-Attitude-Transform,
      performing translation (vec3) and rotation (quaternion as vec4) using
      implementation 2), PAT is per-triangle 2x vec4 in buffer,
      2x uniform matrix (mat4+mat4), 2x uvec4 packed attributes,
      no fragments produced:                   7.750 giga-vertices/s
   Textured Phong and PAT performance - PAT v3 (Position-Attitude-Transform,
      performing translation (vec3) and rotation (quaternion as vec4) using
      implementation 3), PAT is per-triangle 2x vec4 in buffer,
      2x uniform matrix (mat4+mat4), 2x uvec4 packed attributes,
      no fragments produced:                   7.746 giga-vertices/s
   Textured Phong and PAT performance - constant single PAT v2 sourced from
      the same index in buffer (vec3+vec4), 2x uniform matrix (mat4+mat4),
      2x uvec4 packed attributes,
      no fragments produced:                   10.31 giga-vertices/s
   Textured Phong and PAT performance - indexed draw call, per-triangle PAT v2
      in buffer (vec3+vec4), 2x uniform matrix (mat4+mat4), 2x uvec4 packed
      attribute, no fragments produced:        7.076 giga-vertices/s
   Textured Phong and PAT performance - indexed draw call, constant single
      PAT v2 sourced from the same index in buffer (vec3+vec4),
      2x uniform matrix (mat4+mat4), 2x uvec4 packed attributes,
      no fragments produced:                   9.202 giga-vertices/s
   Textured Phong and PAT performance - primitive restart, indexed draw call,
      per-triangle PAT v2 in buffer (vec3+vec4), 2x uniform matrix (mat4+mat4),
      2x uvec4 packed attributes,
      no fragments produced:                   4.935 giga-vertices/s
   Textured Phong and PAT performance - primitive restart, indexed draw call,
      constant single PAT v2 sourced from the same index in buffer (vec3+vec4),
      2x uniform matrix (mat4+mat4), 2x uvec4 packed attributes,
      no fragments produced:                   4.982 giga-vertices/s
   Textured Phong and double precision matrix performance - double precision
      per-triangle matrix in buffer (dmat4), double precision per-scene view
      matrix in uniform (dmat4), both matrices converted to single precision
      before computations, single precision per-scene perspective matrix in
      uniform (mat4), single precision vertex positions, packed attributes
      (2x uvec4), no fragments produced:       2.968 giga-vertices/s
   Textured Phong and double precision matrix performance - double precision
      per-triangle matrix in buffer (dmat4), double precision per-scene view
      matrix in uniform (dmat4), both matrices multiplied in double precision,
      single precision vertex positions, single precision per-scene
      perspective matrix in uniform (mat4), packed attributes (2x uvec4),
      no fragments produced:                   1.566 giga-vertices/s
   Textured Phong and double precision matrix performance - double precision
      per-triangle matrix in buffer (dmat4), double precision per-scene view
      matrix in uniform (dmat4), both matrices multiplied in double precision,
      double precision vertex positions (dvec3), single precision per-scene
      perspective matrix in uniform (mat4), packed attributes (3x uvec4),
      no fragments produced:                   1.629 giga-vertices/s
   Textured Phong and double precision matrix performance using GS - double
      precision per-triangle matrix in buffer (dmat4), double precision
      per-scene view matrix in uniform (dmat4), both matrices multiplied in
      double precision, double precision vertex positions (dvec3), single
      precision per-scene perspective matrix in uniform (mat4), packed
      attributes (3x uvec4),
      no fragments produced:                   621.4 mega-vertices/s

Fragment throughput:
   Single full-framebuffer quad,
      constant color FS:                       63.52 giga-fragments/s
   10x full-framebuffer quad,
      constant color FS:                       66.68 giga-fragments/s
   Four smooth interpolators (4x vec4),
      10x fullscreen quad:                     46.61 giga-fragments/s
   Four flat interpolators (4x vec4),
      10x fullscreen quad:                     49.43 giga-fragments/s
   Four textured phong interpolators (vec3+vec3+vec4+vec2),
      10x fullscreen quad:                     60.35 giga-fragments/s
   Textured Phong, packed uniforms (four smooth interpolators
      (vec3+vec3+vec4+vec2), 4x uniform (material (56 byte) +
      globalAmbientLight (12 byte) + light (64 byte) + sampler2D),
      10x fullscreen quad):                    31.71 giga-fragments/s
   Textured Phong, not packed uniforms (four smooth interpolators
      (vec3+vec3+vec4+vec2), 4x uniform (material (72 byte) +
      globalAmbientLight (12 byte) + light (80 byte) + sampler2D),
      10x fullscreen quad):                    31.44 giga-fragments/s
   Simplified Phong, no texture, no specular (2x smooth interpolator
      (vec3+vec3), 3x uniform (material (vec4+vec4) + globalAmbientLight
      (vec3) + light (48 bytes: position+attenuation+ambient+diffuse)),
      10x fullscreen quad):                    66.17 giga-fragments/s
   Simplified Phong, no texture, no specular, single uniform
      (2x smooth interpolator (vec3+vec3), 1x uniform
      (material+globalAmbientLight+light (vec4+vec4+vec4 + 3x vec4),
      10x fullscreen quad):                    66.17 giga-fragments/s
   Constant color from uniform, 1x uniform (vec4) in FS,
      10x fullscreen quad:                     65.32 giga-fragments/s
   Constant color from uniform, 1x uniform (uint) in FS,
      10x fullscreen quad:                     65.97 giga-fragments/s

Transfer throughput:
   Transfer of consecutive blocks:
      4 bytes: 22.352ns per transfer (0.166665 GiB/s)
      4 bytes: 25.1488ns per transfer (0.14813 GiB/s)
      8 bytes: 24.7584ns per transfer (0.300931 GiB/s)
      16 bytes: 15.9328ns per transfer (0.935251 GiB/s)
      32 bytes: 15.2864ns per transfer (1.9496 GiB/s)
      64 bytes: 15.6768ns per transfer (3.80209 GiB/s)
      128 bytes: 17.0112ns per transfer (7.00769 GiB/s)
      256 bytes: 24.3672ns per transfer (9.78441 GiB/s)
      512 bytes: 45.5391ns per transfer (10.4709 GiB/s)
      1024 bytes: 86.9375ns per transfer (10.9697 GiB/s)
      2048 bytes: 170.438ns per transfer (11.1909 GiB/s)
      4096 bytes: 337.938ns per transfer (11.2882 GiB/s)
      8192 bytes: 681.5ns per transfer (11.195 GiB/s)
      16384 bytes: 1355ns per transfer (11.2611 GiB/s)
      32768 bytes: 2740ns per transfer (11.1378 GiB/s)
      65536 bytes: 5470ns per transfer (11.1582 GiB/s)
      131072 bytes: 10946ns per transfer (11.152 GiB/s)
      262144 bytes: 22468ns per transfer (10.8661 GiB/s)
      524288 bytes: 44544ns per transfer (10.9618 GiB/s)
      1048576 bytes: 89120ns per transfer (10.9578 GiB/s)
      2097152 bytes: 177792ns per transfer (10.9854 GiB/s)
   Transfer of spaced blocks:
      4 bytes: 19.9264ns per transfer (0.186953 GiB/s)
      4 bytes: 22.6688ns per transfer (0.164336 GiB/s)
      8 bytes: 15.9104ns per transfer (0.468284 GiB/s)
      16 bytes: 15.0048ns per transfer (0.993093 GiB/s)
      32 bytes: 15.3664ns per transfer (1.93945 GiB/s)
      64 bytes: 16.2112ns per transfer (3.67676 GiB/s)
      128 bytes: 17.84ns per transfer (6.68214 GiB/s)
      256 bytes: 25.8828ns per transfer (9.21146 GiB/s)
      512 bytes: 49.2891ns per transfer (9.6743 GiB/s)
      1024 bytes: 95.9688ns per transfer (9.93734 GiB/s)
      2048 bytes: 193.156ns per transfer (9.87464 GiB/s)
      4096 bytes: 336.812ns per transfer (11.3259 GiB/s)
      8192 bytes: 687.25ns per transfer (11.1013 GiB/s)
      16384 bytes: 1370.5ns per transfer (11.1337 GiB/s)
      32768 bytes: 2716.5ns per transfer (11.2342 GiB/s)
      65536 bytes: 5469ns per transfer (11.1602 GiB/s)
      131072 bytes: 10846ns per transfer (11.2549 GiB/s)
      262144 bytes: 21948ns per transfer (11.1236 GiB/s)
      524288 bytes: 44032ns per transfer (11.0892 GiB/s)
      1048576 bytes: 88272ns per transfer (11.0631 GiB/s)
      2097152 bytes: 177024ns per transfer (11.0331 GiB/s)

Measurement statistics:
   Triangle throughput measurement time:  1.06 seconds using 19 test rounds.
   Vertex throughput measurement time:    0.0428 seconds using 19 test rounds.
   Attribute and Buffer measurement time: 0.13 seconds using 19 test rounds.
   Transformation measurement time:       0.479 seconds using 19 test rounds.
   Fragment throughput measurement time:  0.0777 seconds using 19 test rounds.
   Transfer throughput measurement time:  0.146 seconds using 19 test rounds.
   Total device time: 1.89 seconds.
   Total real time:   2.05 seconds.
```
