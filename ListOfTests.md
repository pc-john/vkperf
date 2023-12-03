# vkperf tests

Vulkan Performance Measurement Tool perform various tests to measure performance characteristics of particular Vulkan device.
The tests are described in detail in this file.

The list of tests follows:
1. [VS max throughput](#vs-max-throughput)
2. [VS max throughput using indexed draw call](#vs-max-throughput-using-indexed-draw-call)
3. [VS VertexIndex and InstanceIndex forming output](#vs-vertexindex-and-instanceindex-forming-output)
4. [VS VertexIndex and InstanceIndex forming output using indexed draw call](#vs-vertexindex-and-instanceindex-forming-output-using-indexed-draw-call)
5. [GS max throughput when no output is produced](#gs-max-throughput-when-no-output-is-produced)
6. [GS max throughput when single constant triangle is produced](#gs-max-throughput-when-single-constant-triangle-is-produced)
7. [GS max throughput when two constant triangles are produced](#gs-max-throughput-when-two-constant-triangles-are-produced)
8. [Instancing throughput of vkCmdDraw()](#instancing-throughput-of-vkcmddraw)
9. [Instancing throughput of vkCmdDrawIndexed()](#instancing-throughput-of-vkcmddrawindexed)
10. [Instancing throughput of vkCmdDrawIndirect()](#instancing-throughput-of-vkcmddrawindirect)
11. [Instancing throughput of vkCmdDrawIndexedIndirect()](#instancing-throughput-of-vkcmddrawindexedindirect)
12. [Draw command throughput](#draw-command-throughput)
13. [Draw indexed command throughput](#draw-indexed-command-throughput)
14. [VkDrawIndirectCommand processing throughput](#vkdrawindirectcommand-processing-throughput)
15. [VkDrawIndirectCommand processing throughput with stride](#vkdrawindirectcommand-processing-throughput-with-stride)
16. [VkDrawIndexedIndirectCommand processing throughput](#vkdrawindexedindirectcommand-processing-throughput)
17. [VkDrawIndexedIndirectCommand processing throughput with stride](#vkdrawindexedindirectcommand-processing-throughput-with-stride)
18. ... 36. [Attribute and buffer performance](#attribute-and-buffer-performance)
- [Attribute tests](#attribute-tests)
- [Buffer tests](#buffer-tests)
- [Interleaved attribute tests](#interleaved-attribute-tests)
- [Interleaved buffer tests](#interleaved-buffer-tests)
- [Packed data tests](#packed-data-tests)
- [Attribute conversion test](#attribute-conversion-test)
37. ... 51. [Matrix performance](#matrix-performance)
- [Uniform vs buffer vs attribute matrix tests](#uniform-vs-buffer-vs-attribute-matrix-tests)
- [Single whole scene matrix test](#single-whole-scene-matrix-test)
- [Single per-triangle matrix tests](#single-per-triangle-matrix-tests)
- [Three matrices test](#three-matrices-test)
- [Five matrices tests](#five-matrices-tests)
52. ... 69. [Textured Phong performance](#textured-phong-performance)
- [Textured Phong, matrices and four attributes](#textured-phong-matrices-and-four-attributes)
- [Textured Phong, matrices and packed attributes](#textured-phong-matrices-and-packed-attributes)
- [Textured Phong and PAT performance](#textured-phong-and-pat-performance)
- [Textured Phong, PAT, indexed rendering and primitive restart](#textured-phong-pat-indexed-rendering-and-primitive-restart)
- [Textured Phong and double precision matrix performance](#textured-phong-and-double-precision-matrix-performance)
70. [Shared vertex performance](#shared-vertex-performance)
71. [Indexed shared vertex performance](#indexed-shared-vertex-performance)
72. [Triangle strip performance](#triangle-strip-performance)
73. [Indexed triangle strip performance](#indexed-triangle-strip-performance)
74. - 78. [Primitive restart performance](#primitive-restart-performance)
- [Primitive restart performance with single per-scene vkCmdDrawIndexed() call](#primitive-restart-performance-with-single-per-scene-vkcmddrawindexed-call)
- [Primitive restart performance with per-strip vkCmdDrawIndexed() call](#primitive-restart-performance-with-per-strip-vkcmddrawindexed-call)
- [Primitive restart using 2x -1 followed by indexed triangle](#primitive-restart-using-2x-1-followed-by-indexed-triangle)
- [Primitive restart using 5x -1 followed by indexed triangle](#primitive-restart-using-5x-1-followed-by-indexed-triangle)
- [Primitive restart -1 performance, each triangle indices replaced by one -1](#primitive-restart-1-performance-each-triangle-indices-replaced-by-one-1)
79. - 83. [The same vertex processing performance](#the-same-vertex-processing-performance)
- [All 1 index performance, the pipeline uses primitive restart](#all-1-index-performance-the-pipeline-uses-primitive-restart)
- [All 1 index performance, the pipeline uses single per-scene triangle strip](#all-1-index-performance-the-pipeline-uses-single-per-scene-triangle-strip)
- [All 1 index performance, the pipeline uses indexed triangle list](#all-1-index-performance-the-pipeline-uses-indexed-triangle-list)
- [The same vertex triangle strip performance](#the-same-vertex-triangle-strip-performance)
- [The same vertex triangle list performance](#the-same-vertex-triangle-list-performance)


## VS max throughput

Vertex shader maximum throughput test measures number of triangles per second that particular Vulkan device can render.
To get vertex throughput, multiply the result by 3.

The test uses simple vertex shader with constant output. Thus, zero size triangles are produced.
The shader's main() code is as follows:

```c++
void main() {
	gl_Position = vec4(0, 0, 0.5, 1);
}
```

Rendering of the whole scene is performed by a single draw call:

```c++
vkCmdDraw(
	commandBuffer,
	3*numberOfTriangles,  // vertexCount
	1,  // instanceCount
	0,  // firstVertex
	0   // firstInstance
);
```


## VS max throughput using indexed draw call

The test is the same as the [previous test](#vs-max-throughput) except
that it uses indexed draw call:

```c++
vkCmdDrawIndexed(
	commandBuffer,
	3*numberOfTriangles,  // indexCount
	1,  // instanceCount
	0,  // firstIndex
	0,  // vertexOffset
	0   // firstInstance
);
```


## VS VertexIndex and InstanceIndex forming output

The test measures vertex shader throughput with gl_VertexIndex and gl_InstanceIndex 
overhead:

```c++
void main() {
	gl_Position = vec4(0, 0, float(gl_VertexIndex + gl_InstanceIndex) * 1e-20, 1);
}
```

Rendering of the whole scene is performed by a single draw call:

```c++
vkCmdDraw(
	commandBuffer,
	3*numberOfTriangles,  // vertexCount
	1,  // instanceCount
	0,  // firstVertex
	0   // firstInstance
);
```


## VS VertexIndex and InstanceIndex forming output using indexed draw call

The test is the same as the [previous test](#vs-vertexindex-and-instanceindex-forming-output)
except that it uses indexed draw call:

```c++
vkCmdDrawIndexed(
	commandBuffer,
	3*numberOfTriangles,  // indexCount
	1,  // instanceCount
	0,  // firstIndex
	0,  // vertexOffset
	0   // firstInstance
);
```


## GS max throughput when no output is produced

Geometry shader maximum throughput test uses empty geometry shader that produces no output:

```c++
layout(triangles) in;
layout(triangle_strip,max_vertices=3) out;
void main() {
}
```

It is fed by empty vertex shader:

```c++
void main() {
}
```

Rendering of the whole scene is performed by a single draw call:

```c++
vkCmdDraw(
	commandBuffer,
	3*numberOfTriangles,  // vertexCount
	1,  // instanceCount
	0,  // firstVertex
	0   // firstInstance
);
```


## GS max throughput when single constant triangle is produced

The test uses the following geometry shader to produce single constant triangle:

```c++
layout(triangles) in;
layout(triangle_strip,max_vertices=3) out;
void main() {
	gl_Position = vec4(0, 0, 0.5, 1);
	EmitVertex();
	gl_Position = vec4(0, 0, 0.6, 1);
	EmitVertex();
	gl_Position = vec4(0, 1e-10, 0.4, 1);
	EmitVertex();
}
```

It uses empty vertex shader and single vkCmdDraw() call
as in the [previous test](#gs-max-throughput-when-no-output-is-produced).


## GS max throughput when two constant triangles are produced

The test uses the following geometry shader to produce two constant triangles:

```c++
layout(triangles) in;
layout(triangle_strip,max_vertices=6) out;
void main() {
	gl_Position = vec4(0, 0, 0.5, 1);
	EmitVertex();
	gl_Position = vec4(0, 0, 0.6, 1);
	EmitVertex();
	gl_Position = vec4(0, 1e-10, 0.4, 1);
	EmitVertex();
	EndPrimitive();
	gl_Position = vec4(0, 0, 0.7, 1);
	EmitVertex();
	gl_Position = vec4(0, 0, 0.8, 1);
	EmitVertex();
	gl_Position = vec4(0, 1e-10, 0.9, 1);
	EmitVertex();
}
```

It uses empty vertex shader and single vkCmdDraw() call
as in the [previous two tests](#gs-max-throughput-when-no-output-is-produced).


## Instancing throughput of vkCmdDraw()

Instancing throughput test measures instancing performance, e.g. number of instances per second
that particular Vulkan device can process.

The test uses single triangle instanced very many times.
Thus, the triangle throughput is equal to instance throughput.

The whole scene is rendered using a single vkCmdDraw() call:

```c++
vkCmdDraw(
	commandBuffer,
	3,  // vertexCount
	numberOfTriangles,  // instanceCount
	0,  // firstVertex
	0   // firstInstance
);
```

The vertex shader outputs constant coordinates, thus producing zero-sized triangles:

```c++
void main() {
	gl_Position = vec4(0, 0, 0.5, 1);
}
```


## Instancing throughput of vkCmdDrawIndexed()

The test is the same as the [previous test](#instancing-throughput-of-vkcmddraw)
except that it uses indexed draw call:

```c++
vkCmdDrawIndexed(
	commandBuffer,
	3,  // indexCount
	numberOfTriangles,  // instanceCount
	0,  // firstIndex
	0,  // vertexOffset
	0   // firstInstance
);
```


## Instancing throughput of vkCmdDrawIndirect()

Instancing throughput test of vkCmdDrawIndirect() uses single triangle instanced
many times in single VkDrawIndirectCommand struct with the following content:

```c++
indirectBufferPtr->vertexCount = 3;
indirectBufferPtr->instanceCount = numberOfTriangles;
indirectBufferPtr->firstVertex = 0;
indirectBufferPtr->firstInstance = 0;
```

The VkDrawIndirectCommand structure is processed by a single vkCmdDrawIndirect() call:

```c++
vkCmdDrawIndirect(
	commandBuffer,
	indirectBuffer,  // buffer
	0,  // offset
	1,  // drawCount
	sizeof(VkDrawIndirectCommand)  // stride
);
```

The vertex shader outputs constant coordinates, thus producing zero-sized triangles:

```c++
void main() {
	gl_Position = vec4(0, 0, 0.5, 1);
}
```


## Instancing throughput of vkCmdDrawIndexedIndirect()

Instancing throughput test of vkCmdDrawIndexedIndirect() uses single triangle instanced
many times in single VkDrawIndexedIndirectCommand struct with the following content:

```c++
	indirectIndexedBufferPtr->indexCount = 3;
	indirectIndexedBufferPtr->instanceCount = numTriangles;
	indirectIndexedBufferPtr->firstIndex = 0;
	indirectIndexedBufferPtr->vertexOffset = 0;
	indirectIndexedBufferPtr->firstInstance = 0;
```

The VkDrawIndexedIndirectCommand structure is processed by a single vkCmdDrawIndirect() call:

```c++
vkCmdDrawIndexedIndirect(
	commandBuffer,
	indirectBuffer,  // buffer
	0,  // offset
	1,  // drawCount
	sizeof(VkDrawIndexedIndirectCommand)  // stride
);
```

The vertex shader outputs constant coordinates as in the [previous test](#instancing-throughput-of-vkcmddrawindirect).


## Draw command throughput

Draw command throughput test measures performance of vkCmdDraw().
Each draw call renders single triangle.
Thus, triangle throughput is equal to vkCmdDraw() throughput.

Draw code is equal to the following one:

```c++
for(uint32_t i=0; i<numberOfTriangles; i++)
	vkCmdDraw(
		commandBuffer,
		3,  // vertexCount
		1,  // instanceCount
		3*i,  // firstVertex
		0   // firstInstance
	);
```

The vertex shader outputs constant coordinates, thus producing zero-sized triangles:

```c++
void main() {
	gl_Position = vec4(0, 0, 0.5, 1);
}
```


## Draw indexed command throughput

The test is the same as the [previous test](#draw-command-throughput)
except that it uses indexed draw call:

```c++
for(uint32_t i=0; i<numberOfTriangles; i++)
	vkCmdDrawIndexed(
		commandBuffer,
		3,  // indexCount
		1,  // instanceCount
		3*i,  // firstIndex
		0,  // vertexOffset
		0   // firstInstance
	);
```


## VkDrawIndirectCommand processing throughput

VkDrawIndirectCommand processing throughput test measures
the number of VkDrawIndirectCommand structures processed per second.
Each VkDrawIndirectCommand contains single triangle,
thus triangle throughput is equal to VkDrawIndirectCommand processing throughput.
The content of VkDrawIndirectCommand is as follows:

```c++
indirectBufferPtr[i].vertexCount = 3;
indirectBufferPtr[i].instanceCount = 1;
indirectBufferPtr[i].firstVertex = indexOfTriangle * 3;
indirectBufferPtr[i].firstInstance = 0;
```

The vertex shader outputs constant coordinates, thus producing zero-sized triangles:

```c++
void main() {
	gl_Position = vec4(0, 0, 0.5, 1);
}
```

The rendering command is as follows:

```c++
vkCmdDrawIndirect(
	commandBuffer,
	indirectBuffer.get(),  // buffer
	0,  // offset
	numTriangles,  // drawCount
	sizeof(VkDrawIndirectCommand)  // stride
);
```


## VkDrawIndirectCommand processing throughput with stride

The test is the same as the [previous test](#vkdrawindirectcommand-processing-throughput)
except that it uses greater stride value:

```c++
vkCmdDrawIndirect(
	commandBuffer,
	indirectBuffer.get(),  // buffer
	0,  // offset
	numTriangles,  // drawCount
	32  // stride
);
```


## VkDrawIndexedIndirectCommand processing throughput

VkDrawIndexedIndirectCommand processing throughput test measures
the number of VkDrawIndexedIndirectCommand structures processed per second.
Each VkDrawIndexedIndirectCommand contains single triangle,
thus triangle throughput is equal to VkDrawIndexedIndirectCommand processing throughput.
The content of VkDrawIndexedIndirectCommand is as follows:

```c++
indirectIndexedBufferPtr[i].indexCount = 3;
indirectIndexedBufferPtr[i].instanceCount = 1;
indirectIndexedBufferPtr[i].firstIndex = indexOfTriangle * 3;
indirectIndexedBufferPtr[i].vertexOffset = 0;
indirectIndexedBufferPtr[i].firstInstance = 0;
```

The vertex shader outputs constant coordinates, thus producing zero-sized triangles:

```c++
void main() {
	gl_Position = vec4(0, 0, 0.5, 1);
}
```

The rendering command is as follows:

```c++
vkCmdDrawIndexedIndirect(
	commandBuffer,
	indirectIndexedBuffer.get(),  // buffer
	0,  // offset
	numTriangles,  // drawCount
	sizeof(VkDrawIndexedIndirectCommand)  // stride
);
```


## VkDrawIndexedIndirectCommand processing throughput with stride

The test is the same as the [previous test](#vkdrawindexedindirectcommand-processing-throughput)
except that it uses greater stride value:

```c++
vkCmdDrawIndexedIndirect(
	commandBuffer,
	indirectIndexedBuffer.get(),  // buffer
	0,  // offset
	numTriangles,  // drawCount
	32  // stride
);
```


## Attribute and buffer performance

All the tests in this section measure triangle rendering performance while using
one to four attributes or buffers in vertex shader.

The tests in this section might answer the questions like:
- whether attributes or buffers are the faster option,
- what is the impact of number of attributes on the performance,
- how vec3 for vertex positions competes with the performance of vec4 vertex positions,
- whether interleaved attributes or continuous buffer data of more than 16 bytes are fast options,
- whether packing data to minimize vertex shader memory bandwidth is worthwhile.


### Attribute tests

For attribute tests, the performance of one to four vec4 attributes is measured.
The vertex shader looks like:

```c++
layout(location=0) in vec4 inPosition;
[layout(location=1) in vec4 attribute1; // contains vec4(-2,-2,-2,-2)]
[layout(location=2) in vec4 attribute2; // contains vec4(-2,-2,-2,-2) or vec4(1,1,1,1)]
[layout(location=3) in vec4 attribute3; // contains vec4(4,4,4,4) or vec4(1,1,1,1)]

void main() {
	gl_Position = inPosition [+attribute1] [+attribute2] [+attribute3] [+vec4(2.,2.,2.,2.)];
}
```

Depending on the number of attributes, the shader code differs slightly.
However, sum of all attributes except inPosition is made in a way to always be 0,0,0,0.
Thus, just inPosition affects the resulting vertex position.
The values of inPosition specify tiny triangles in between pixel sampling locations
distributed roughly across the whole framebuffer.
So, they do not produce any fragments in the rasterizer.

This is used in the following tests:
- One attribute performance - 1x vec4 attribute
- Two attributes performance - 2x vec4 attribute
- Four attributes performance - 4x vec4 attribute


### Buffer tests

For buffer tests, the performance of one to four vec4 buffers is measured.
The vertex shader looks like:

```c++
layout(std430,binding=0) restrict readonly buffer CoordinateBuffer {
	vec4 coordinateBuffer[];
};
[layout(std430,binding=1) restrict readonly buffer Data1Buffer {
	vec4 data1[]; // contains vec4(-2,-2,-2,-2)
};]
[layout(std430,binding=2) restrict readonly buffer Data2Buffer {
	vec4 data2[]; // contains vec4(-2,-2,-2,-2)
};]
[layout(std430,binding=3) restrict readonly buffer Data3Buffer {
	vec4 data3[]; // contains vec4(4,4,4,4)
};]

void main() {
	gl_Position = coordinateBuffer[gl_VertexIndex] [+data1[gl_VertexIndex]]
	              [+data2[gl_VertexIndex]] [+data3[gl_VertexIndex]];
}
```

Depending on the number of buffers, the shader code differs slightly.
However, sum of all buffers except coordinateBuffer is made in a way to always be 0,0,0,0.
Thus, just coordinateBuffer affects the resulting vertex position.
The values in coordinateBuffer specify tiny triangles in between pixel sampling locations
distributed roughly across the whole framebuffer.
So, they do not produce any fragments in the rasterizer.

This is used in the following tests:
- One buffer performance - 1x vec4 buffer
- One buffer performance - 1x vec3 buffer
- Two buffers performance - 2x vec4 buffer
- Two buffers performance - 2x vec3 buffer
- Four buffers performance - 4x vec4 buffer
- Four buffers performance - 4x vec3 buffer


### Interleaved attribute tests

Interleaved attribute tests are using vertex input state that points all attributes
to the consecutive places in the same buffer:

```c++
vk::PipelineVertexInputStateCreateInfo{
	vk::PipelineVertexInputStateCreateFlags(),  // flags
	1,  // vertexBindingDescriptionCount
	array<const vk::VertexInputBindingDescription,1>{  // pVertexBindingDescriptions
		vk::VertexInputBindingDescription(
			0,  // binding
			16*sizeof(float),  // stride
			vk::VertexInputRate::eVertex  // inputRate
		),
	}.data(),
	4,  // vertexAttributeDescriptionCount
	array<const vk::VertexInputAttributeDescription,4>{  // pVertexAttributeDescriptions
		vk::VertexInputAttributeDescription(
			0,  // location
			0,  // binding
			vk::Format::eR32G32B32A32Sfloat,  // format
			0   // offset
		),
		vk::VertexInputAttributeDescription(
			1,  // location
			0,  // binding
			vk::Format::eR32G32B32A32Sfloat,  // format
			16  // offset
		),
		vk::VertexInputAttributeDescription(
			2,  // location
			0,  // binding
			vk::Format::eR32G32B32A32Sfloat,  // format
			32  // offset
		),
		vk::VertexInputAttributeDescription(
			3,  // location
			0,  // binding
			vk::Format::eR32G32B32A32Sfloat,  // format
			48  // offset
		),
	}.data()
});
```

Vertex shaders are the same as standard attribute tests.

This is used in the following tests:
- Two interleaved attributes performance - 2x vec4
- Four interleaved attributes performance - 4x vec4


### Interleaved buffer tests

Interleaved buffer tests are using vertex shaders
that read various amount of interleaved data from
the consecutive places in the same buffer.

```c++
struct VertexData {
	vec4 pos;
	vec4 data1;  // contains vec4(-2,-2,-2,-2)
[	vec4 data2;  // contains vec4(-2,-2,-2,-2)
	vec4 data3;  // contains vec4(4,4,4,4)]
};

layout(std430, binding=0) restrict readonly buffer Buffer {
	VertexData vertexData[];
};

void main() {
	VertexData d = vertexData[gl_VertexIndex];
	gl_Position = d.pos + d.data1 [+d.data2+d.data3] [+vec4(2.,2.,2.,2.)];
}
```

Vertex shader produces tiny triangles in between pixel sampling locations
distributed roughly across the whole framebuffer.
So, they do not produce any fragments in the rasterizer.

This is used in the following tests:
- Two interleaved buffers performance - 2x vec4
- Four interleaved buffers performance - 4x vec4


### Packed data tests

These tests use one or two uvec4 buffers which are unpacked into four attributes.

The first buffer typically contains x, y, and z coordinates as 4-byte floats.
The fourth component is usually occupied
by two half floats - w coordinate and normal's z component.

The second buffer typically contains texture u and v coordinates stored
as 4-byte floats, normal's x and y component stored as half floats,
and color stored as uint.

The unpacked data produce tiny triangles in between pixel sampling locations
distributed roughly across the whole framebuffer.
So, they do not produce any fragments in the rasterizer.

#### Packed attribute performance - 2x uvec4 attribute unpacked

The test uses two uvec4 attributes that are unpacked to four attributes:

```c++
layout(location=0) in uvec4 packedData1;  // 0: float posX, 1: float posY, 2: float posZ, 3: half normalZ + half posW
layout(location=1) in uvec4 packedData2;  // 0: float texU, 1: float texV, 2: half normalX + half normalY, 3: uint color

void main() {
	vec2 extra = unpackHalf2x16(packedData1.w);
	vec4 position = vec4(uintBitsToFloat(packedData1.xyz), extra.y);
	vec3 normal = vec3(unpackHalf2x16(packedData2.z), extra.x);
	vec4 color = unpackUnorm4x8(packedData2.w);
	vec2 texCoord = uintBitsToFloat(packedData2.xy);
	gl_Position = position * color * vec4(normal, 1) * vec4(texCoord, 1, 1);
}
```

More details about packed data can be found in [Packed data tests](#packed-data-tests) section.

#### Packed buffer performance - 1x buffer using 32-byte struct unpacked

The test uses 32-byte struct stored in one buffer
that is unpacked into four attributes:

```c++
struct PackedData {
	vec3 position;
	uint packedColor;
	vec2 texCoord;
	uint extra1;  // half normalX + half normalY
	uint extra2;  // half normalZ + half posW
};

layout(std430,binding=0) restrict readonly buffer PackedDataBuffer {
	PackedData packedData[];
};

void main() {
	PackedData d = packedData[gl_VertexIndex];
	vec2 normalZandPosW = unpackHalf2x16(d.extra2);
	gl_Position = vec4(d.position, normalZandPosW.y) *
	              unpackUnorm4x8(d.packedColor) *
	              vec4(unpackHalf2x16(d.extra1), normalZandPosW.x, 1) *
	              vec4(d.texCoord, 1, 1);
}
```

More details about packed data can be found in [Packed data tests](#packed-data-tests) section.

#### Packed buffer performance - 2x uvec4 buffers unpacked

The test uses two uvec4 buffers that are unpacked into four attributes:

```c++
layout(std430,binding=0) restrict readonly buffer PackedDataBuffer1 {
	uvec4 packedData1[];  // 0: float posX, 1: float posY, 2: float posZ, 3: half normalZ + half posW
};
layout(std430,binding=1) restrict readonly buffer PackedDataBuffer2 {
	uvec4 packedData2[];  // 0: float texU, 1: float texV, 2: half normalX + half normalY, 3: uint color
};

void main() {
	uvec4 data1 = packedData1[gl_VertexIndex];
	uvec4 data2 = packedData2[gl_VertexIndex];
	vec2 extra = unpackHalf2x16(data1.w);
	vec4 position = vec4(uintBitsToFloat(data1.xyz), extra.y);
	vec3 normal = vec3(unpackHalf2x16(data2.z), extra.x);
	vec4 color = unpackUnorm4x8(data2.w);
	vec2 texCoord = uintBitsToFloat(data2.xy);
	gl_Position = position * color * vec4(normal, 1) * vec4(texCoord, 1, 1);
}
```

More details about packed data can be found in [Packed data tests](#packed-data-tests) section.

#### Packed buffer performance - 2x buffer using 16-byte struct unpacked

The test uses two structs stored in two buffers that are unpacked into four attributes:

```c++
struct PackedData1 {
	vec3 position;
	uint extra1;  // half normalZ + half posW
};
struct PackedData2 {
	vec2 texCoord;
	uint extra2;  // half normalX + half normalY
	uint packedColor;
};

layout(std430,binding=0) restrict readonly buffer PackedDataBuffer1 {
	PackedData1 packedData1[];
};
layout(std430,binding=1) restrict readonly buffer PackedDataBuffer2 {
	PackedData2 packedData2[];
};

void main() {
	PackedData1 d1 = packedData1[gl_VertexIndex];
	PackedData2 d2 = packedData2[gl_VertexIndex];
	vec2 normalZandPosW = unpackHalf2x16(d1.extra1);
	gl_Position = vec4(d1.position, normalZandPosW.y) *
	              unpackUnorm4x8(d2.packedColor) *
	              vec4(unpackHalf2x16(d2.extra2), normalZandPosW.x, 1) *
	              vec4(d2.texCoord, 1, 1);
}
```

More details about packed data can be found in [Packed data tests](#packed-data-tests) section.

#### Packed buffer performance - 2x buffer using 16-byte struct read multiple times and unpacked

The test uses two structs stored in two buffers that are read multiple times
and unpacked into four attributes:

```c++
struct PackedData1 {
	vec3 position;
	uint extra1;  // half normalZ + half posW
};
struct PackedData2 {
	vec2 texCoord;
	uint extra2;  // half normalX + half normalY
	uint packedColor;
};

layout(std430,binding=0) restrict readonly buffer PackedDataBuffer1 {
	PackedData1 packedData1[];
};
layout(std430,binding=1) restrict readonly buffer PackedDataBuffer2 {
	PackedData2 packedData2[];
};

void main() {
	vec2 normalZandPosW = unpackHalf2x16(packedData1[gl_VertexIndex].extra1);
	gl_Position = vec4(packedData1[gl_VertexIndex].position, normalZandPosW.y) *
	              unpackUnorm4x8(packedData2[gl_VertexIndex].packedColor) *
	              vec4(unpackHalf2x16(packedData2[gl_VertexIndex].extra2), normalZandPosW.x, 1) *
	              vec4(packedData2[gl_VertexIndex].texCoord, 1, 1);
}
```

More details about packed data can be found in [Packed data tests](#packed-data-tests) section.

### Attribute conversion test

It is used in the test:
- Four attributes performance - 2x vec4 and 2x uint attribute

The code of the test is the same as
"Four attributes performance - 4x vec4 attribute" test in [Attribute tests](#attribute-tests) section
with the exception that two of four buffers are not using VK_FORMAT_R32G32B32A32_SFLOAT
but VK_FORMAT_R8G8B8A8_UNORM in VkPipelineVertexInputStateCreateInfo.
Thus, the data conversion is needed on the shader input.
The measurement of the conversion overhead is the focus of this test.

```c++
array<const vk::VertexInputAttributeDescription,4>{  // pVertexAttributeDescriptions
	vk::VertexInputAttributeDescription(
		0,  // location
		0,  // binding
		vk::Format::eR32G32B32A32Sfloat,  // format
		0   // offset
	),
	vk::VertexInputAttributeDescription(
		1,  // location
		1,  // binding
		vk::Format::eR32G32B32A32Sfloat,  // format
		0   // offset
	),
	vk::VertexInputAttributeDescription(
		2,  // location
		2,  // binding
		vk::Format::eR8G8B8A8Unorm,  // format
		0   // offset
	),
	vk::VertexInputAttributeDescription(
		3,  // location
		3,  // binding
		vk::Format::eR8G8B8A8Unorm,  // format
		0   // offset
	),
}.data()
```


## Matrix performance

The matrix performance tests use one to five matrices in vertex or geometry shader.
Matrices are read from uniform variable, buffer, attribute, shader constant or specialization constant.
The matrices are either per-triangle or per-scene.
Per-triangle matrices are read from different memory locations.
Per-scene matrices are either constants, specialization constants or they are read from
the same memory location by each shader invocation.

### Uniform vs buffer vs attribute matrix tests

The three tests compare performance of uniform matrix, per-triangle matrix read from buffer,
and per-triangle matrix read using attribute:

- Matrix performance - one matrix as uniform for all triangles
 
```c++
layout(location=0) in vec4 inPosition;

layout(binding=0) uniform UniformBufferObject {
	mat4 modelView;
};

void main() {
	gl_Position = modelView * inPosition;
}
```

- Matrix performance - per-triangle matrix in buffer

```c++
layout(location=0) in vec4 inPosition;

layout(std430,binding=0) restrict readonly buffer TransformationMatrix {
	mat4 transformationMatrix[];
};

void main() {
	gl_Position = transformationMatrix[gl_VertexIndex/3] * inPosition;
}
```

- Matrix performance - per-triangle matrix in attribute

```c++
layout(location=0) in vec4 inPosition;
layout(location=1) in mat4 transformationMatrix;

void main() {
	gl_Position = transformationMatrix * inPosition;
}
```

### Single per-scene matrix test

Name of the test:
- Matrix performance - one matrix in buffer for all triangles and two packed attributes

This test shows the performance benefit of using per-scene matrix
over to per-triangle matrices.
Per-triangle matrices are measured in the [tests bellow](#single-per-triangle-matrix-tests).

```c++
layout(location=0) in uvec4 packedData1;  // 0: float posX, 1: float posY, 2: float posZ, 3: half normalZ + half posW
layout(location=1) in uvec4 packedData2;  // 0: float texU, 1: float texV, 2: half normalX + half normalY, 3: uint color

layout(std430,binding=0) restrict readonly buffer TransformationMatrix {
	mat4 transformationMatrix[];
};

void main() {
	vec2 extra = unpackHalf2x16(packedData1.w);
	vec4 position = vec4(uintBitsToFloat(packedData1.xyz), extra.y);
	vec3 normal = vec3(unpackHalf2x16(packedData2.z), extra.x);
	vec4 color = unpackUnorm4x8(packedData2.w);
	vec2 texCoord = uintBitsToFloat(packedData2.xy);
	gl_Position = transformationMatrix[0] * position * color * vec4(normal, 1) * vec4(texCoord, 1, 1);
}
```

### Single per-triangle matrix tests

The tests read a matrix from a buffer in vertex or geometry shader.
They also process 2x uvec4 packed attributes or four not packed attributes.

The vertex shader code is similar to the one used in the [previous section](#single-per-scene-matrix-test)
except that it does not use per-scene matrix but per-triangle matrix.

Such approach is used in the following tests:
- Matrix performance - per-triangle matrix in buffer and two packed attributes
- Matrix performance - per-triangle matrix in buffer and two packed buffers
- Matrix performance - GS reads per-triangle matrix from buffer and two packed buffers
- Matrix performance - per-triangle matrix in buffer and four attributes

### Three matrices test

Three matrices test represents a typical rendering setup when only vertex positions are processed,
or when normal transformation matrices can be derived from view and model matrices.

It is used in the following test:
- Matrix performance - 1x per-triangle matrix in buffer, 2x uniform matrix and and two packed attributes

### Five matrices tests

Five matrices tests represent a typical rendering setup when vertex positions and normals are processed.
Positions utilize perspective, view and model matrix, e.g. 3x mat4.
Normals need transposed inverse of view matrix and of model matrix, e.g. 2x mat3, unless further optimizations are deployed.

This is used in the following tests:
- Matrix performance - 2x per-triangle matrix (mat4+mat3) in buffer, 3x uniform matrix (mat4+mat4+mat3) and two packed attributes
- Matrix performance - 2x per-triangle matrix (mat4+mat3) in buffer, 2x non-changing matrix in push constants (mat4+mat4), 1x constant matrix (mat3) and two packed attributes
- Matrix performance - 2x per-triangle matrix (mat4+mat3) in buffer, 2x non-changing matrix (mat4+mat4) in specialization constants, 1x constant matrix (mat3) and two packed attributes
- Matrix performance - 2x per-triangle matrix (mat4+mat3) in buffer, 3x constant matrix (mat4+mat4+mat3) defined by VS code and two packed attribute
- Matrix performance - GS five matrices processing, 2x per-triangle matrix (mat4+mat3) in buffer, 3x uniform matrix (mat4+mat4+mat3) and 2x packed attribute passed through VS
- Matrix performance - GS five matrices processing, 2x per-triangle matrix (mat4+mat3) in buffer, 3x uniform matrix (mat4+mat4+mat3) and 2x packed data read from buffer in GS

All the tests are variations on the following shader code:
```c++
layout(location=0) in uvec4 packedData1;  // 0: float posX, 1: float posY, 2: float posZ, 3: half normalZ + half posW
layout(location=1) in uvec4 packedData2;  // 0: float texU, 1: float texV, 2: half normalX + half normalY, 3: uint color

layout(std430,binding=0) restrict readonly buffer TransformationMatrix {
	mat4 transformationMatrix[];
};

layout(std430,binding=1) restrict readonly buffer NormalMatrix {
	mat3 normalMatrix[];
};

layout(binding=2) uniform UniformBufferObject {
	mat4 viewMatrix;
	mat4 projectionMatrix;
	mat3 normalViewMatrix;
};

out gl_PerVertex {
	vec4 gl_Position;
};

void main() {
	vec2 extra = unpackHalf2x16(packedData1.w);
	vec4 position = vec4(uintBitsToFloat(packedData1.xyz), extra.y);
	vec3 normal = vec3(unpackHalf2x16(packedData2.z), extra.x); 
	vec4 color = unpackUnorm4x8(packedData2.w);
	vec2 texCoord = uintBitsToFloat(packedData2.xy);
	gl_Position = projectionMatrix * viewMatrix * transformationMatrix[gl_VertexIndex/3] * position *
	              color * vec4(normalViewMatrix*normalMatrix[gl_VertexIndex/3]*normal, 1) * vec4(texCoord, 1, 1);
}
```


## Textured Phong performance

These tests simulate textured Phong rendering. All the necessary inputs are fed into the VS,
e.g. positions, normals, color and texture coordinates in the form of packed attributes (2x uvec4)
and all transformation matrices (3 or 5 matrices). Phong lighting is usually computed in FS
that is never run in our tests to measure pure triangle throughput.
So, it is processing of transformations and making all attributes ready that are main focus of these tests.

### Textured Phong, matrices and four attributes

Tests using four attributes use vertex shader similar to the following one:

```c++
layout(location=0) in vec4 inPosition;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec4 inColor;
layout(location=3) in vec2 inTexCoord;

layout(std430,binding=0) restrict readonly buffer ModelMatrix {
	mat4 modelMatrix[];
};

layout(std430,binding=1) restrict readonly buffer NormalMatrix {
	mat3 normalMatrix[];
};

layout(binding=2) uniform UniformBufferObject {
	mat4 viewMatrix;
	mat4 projectionMatrix;
	mat3 normalViewMatrix;
};

layout(location=0) out vec3 eyePosition;
layout(location=1) out vec3 eyeNormal;
layout(location=2) out vec4 outColor;
layout(location=3) out vec2 outTexCoord;

void main()
{
	// compute outputs
	vec4 eyePosition4 = viewMatrix * modelMatrix[gl_VertexIndex/3] * inPosition;
	gl_Position = projectionMatrix * eyePosition4;
	eyePosition = eyePosition4.xyz;
	eyeNormal = normalViewMatrix * normalMatrix[gl_VertexIndex/3] * inNormal;
	outColor = inColor;
	outTexCoord = inTexCoord;
}
```

Such approach is used in the following tests:
- Textured Phong and Matrix performance - 2x per-triangle matrix in buffer (mat4+mat3), 3x uniform matrix (mat4+mat4+mat3) and four attributes (vec4f32+vec3f32+vec4u8+vec2f32)
- Textured Phong and Matrix performance - 1x per-triangle matrix in buffer (mat4), 2x uniform matrix (mat4+mat4) and four attributes (vec4f32+vec3f32+vec4u8+vec2f32)

### Textured Phong, matrices and packed attributes

Tests using packed attributes use vertex shader similar to the following one:

```c++
layout(location=0) in uvec4 packedData1;  // 0: float posX, 1: float posY, 2: float posZ, 3: half normalZ + half posW
layout(location=1) in uvec4 packedData2;  // 0: float texU, 1: float texV, 2: half normalX + half normalY, 3: uint color

layout(std430,binding=0) restrict readonly buffer ModelMatrix {
	layout(column_major) mat4 modelMatrix[];
};

layout(binding=1) uniform UniformBufferObject {
	mat4 viewMatrix;
	mat4 projectionMatrix;
};

layout(location=0) out vec3 eyePosition;
layout(location=1) out vec3 eyeNormal;
layout(location=2) out vec4 color;
layout(location=3) out vec2 texCoord;


void main() {

	// unpack data
	vec2 extra = unpackHalf2x16(packedData1.w);
	vec4 position = vec4(uintBitsToFloat(packedData1.xyz), extra.y);
	vec3 normal = vec3(unpackHalf2x16(packedData2.z), extra.x);

	// compute outputs
	mat4 m = modelMatrix[gl_VertexIndex/3];
	vec4 eyePosition4 = viewMatrix * m * position;
	gl_Position = projectionMatrix * eyePosition4;
	eyePosition = eyePosition4.xyz;
	eyeNormal = mat3(viewMatrix) * mat3(m) * normal;
	color = unpackUnorm4x8(packedData2.w);
	texCoord = uintBitsToFloat(packedData2.xy);

}
```

With some variations, it is used in the following tests:
- Textured Phong and Matrix performance - 1x per-triangle matrix in buffer (mat4), 2x uniform matrix (mat4+mat4) and 2x packed attribute
- Textured Phong and Matrix performance - 1x per-triangle row-major matrix in buffer (mat4), 2x uniform not-row-major matrix (mat4+mat4), 2x packed attribute
- Textured Phong and Matrix performance - 1x per-triangle mat4x3 matrix in buffer, 2x uniform matrix (mat4+mat4) and 2x packed attribute
- Textured Phong and Matrix performance - 1x per-triangle row-major mat4x3 matrix in buffer, 2x uniform matrix (mat4+mat4), 2x packed attribute


### Textured Phong and PAT performance

These tests simulate textured Phong rendering while using Position-Attitude-Transform (PAT) instead of matrices for tranformation.
PAT is composed of translation of vertex positions and of object attitude, e.g. rotation.
In our tests, the rotation is given by quaternion (vec4) and
translation is specified by vec3 while stored as vec4, leaving one float for uniform scale or w-component.

We use three different quaternion implementations marked PAT v1, PAT v2 and PAT v3.

The VS code is similar to the following one:

```c++
layout(location=0) in uvec4 packedData1;  // 0: float posX, 1: float posY, 2: float posZ, 3: half normalZ + half posW
layout(location=1) in uvec4 packedData2;  // 0: float texU, 1: float texV, 2: half normalX + half normalY, 3: uint color

layout(binding=0) restrict readonly buffer ModelMatrix {
	vec4 modelPAT[];  // model Position and Attitude Transformation, each transformation is composed
	                  // of two vec4 - the first one is quaternion and the second translation
};

layout(binding=1) uniform UniformBufferObject {
	mat4x4 viewMatrix;
	mat4 projectionMatrix;
};

layout(location=0) out vec3 eyePosition;
layout(location=1) out vec3 eyeNormal;
layout(location=2) out vec4 color;
layout(location=3) out vec2 texCoord;


void main() {

	// unpack data
	vec2 extra = unpackHalf2x16(packedData1.w);
	vec3 position = uintBitsToFloat(packedData1.xyz);
	vec3 normal = vec3(unpackHalf2x16(packedData2.z), extra.x);

	// compute outputs
	uint i = gl_VertexIndex/3*2;
	vec4 q = modelPAT[i+0];
	vec3 t = modelPAT[i+1].xyz;
	vec3 worldPosition = position + (2 * cross(q.xyz, cross(q.xyz, position) + (q.w * position))) + t;
	eyePosition = mat3(viewMatrix) * worldPosition  +viewMatrix[3].xyz;
	gl_Position = projectionMatrix * vec4(eyePosition, 1);
	vec3 worldNormal = normal + (2 * cross(q.xyz, cross(q.xyz, normal) + (q.w * normal)));
	eyeNormal = mat3(viewMatrix) * worldNormal;
	color = unpackUnorm4x8(packedData2.w);
	texCoord = uintBitsToFloat(packedData2.xy);

}
```

Such approach is used in the following tests:
- Textured Phong and PAT performance - PAT v1 (Position-Attitude-Transform, performing translation (vec3) and rotation (quaternion as vec4) using implementation 1), PAT is per-triangle 2x vec4 in buffer, 2x uniform matrix (mat4+mat4), 2x packed attribute
- Textured Phong and PAT performance - PAT v2 (Position-Attitude-Transform, performing translation (vec3) and rotation (quaternion as vec4) using implementation 2), PAT is per-triangle 2x vec4 in buffer, 2x uniform matrix (mat4+mat4), 2x packed attribute
- Textured Phong and PAT performance - PAT v3 (Position-Attitude-Transform, performing translation (vec3) and rotation (quaternion as vec4) using implementation 3), PAT is per-triangle 2x vec4 in buffer, 2x uniform matrix (mat4+mat4), 2x packed attribute
- Textured Phong and PAT performance - constant single PAT v2 sourced from the same index in buffer (2x vec4), 2x uniform matrix (mat4+mat4), 2x packed attribute

### Textured Phong, PAT, indexed rendering and primitive restart

The tests based on [Textured Phong and PAT](#textured-phong-and-pat-performance)
measure the performance of indexed rendering and primitive restart.
Indexed rendering uses monotonically increasing indices,
making each following index greater by one.
Primitive restart tests appends -1 after each triangle.
These are used in the following tests:

- Textured Phong and PAT performance - indexed draw call, per-triangle PAT v2 in buffer (2x vec4), 2x uniform matrix (mat4+mat4), 2x packed attribute
- Textured Phong and PAT performance - indexed draw call, constant single PAT v2 sourced from the same index in buffer (2x vec4), 2x uniform matrix (mat4+mat4), 2x packed attribute
- Textured Phong and PAT performance - primitive restart, indexed draw call, per-triangle PAT v2 in buffer (2x vec4), 2x uniform matrix (mat4+mat4), 2x packed attribute
- Textured Phong and PAT performance - primitive restart, indexed draw call, constant single PAT v2 sourced from the same index in buffer (2x vec4), 2x uniform matrix (mat4+mat4), 2x packed attribute

### Textured Phong and double precision matrix performance

Transformations in doubles are sometimes useful for scientific simulations or for visualization of large models.
The tests are using double precision model and view matrices. Vertex positions are either single precision
or double precision. Single precision vertex positions might be sufficient for small scene objects
that only need double precision for placing the object into a very large scene.
Double precision vertex positions might be useful for large scene objects or when very high precision is desired.

Typical VS code looks like:

```c++
layout(location=0) in uvec4 packedData1;  // 0-1: doubleX, 2-3: doubleY
layout(location=1) in uvec4 packedData2;  // 0-1: doubleZ, 2: half normalX + half normalY, 3: half normalZ + half posW
layout(location=2) in uvec4 packedData3;  // 0: float texU, 1: float texV, 2: uint color

layout(std430,binding=0) restrict readonly buffer ModelMatrix {
	dmat4 modelMatrix[];
};

layout(binding=1) uniform UniformBufferObject {
	dmat4 viewMatrix;
	mat4 projectionMatrix;
};

layout(location=0) out vec3 eyePosition;
layout(location=1) out vec3 eyeNormal;
layout(location=2) out vec4 color;
layout(location=3) out vec2 texCoord;

void main() {

	// unpack data
	vec2 extra = unpackHalf2x16(packedData2.w);
	dvec4 position = dvec4(packDouble2x32(packedData1.xy), packDouble2x32(packedData1.zw),
	                       packDouble2x32(packedData2.xy), extra.y);
	vec3 normal = vec3(unpackHalf2x16(packedData2.z), extra.x);

	// compute outputs
	dmat4 m = modelMatrix[gl_VertexIndex/3];
	vec4 eyePosition4 = vec4(viewMatrix * m * position);
	gl_Position = projectionMatrix * eyePosition4;
	eyePosition = eyePosition4.xyz;
	eyeNormal = mat3(viewMatrix) * mat3(m) * normal;
	color = unpackUnorm4x8(packedData3.z);
	texCoord = uintBitsToFloat(packedData3.xy);

}
```

List of tests that use this approach:
- Textured Phong and double precision matrix performance - double precision per-triangle matrix in buffer (dmat4),
  double precision per-scene view matrix in uniform (dmat4), both matrices converted to single precision before computations,
  single precision per-scene perspective matrix in uniform (mat4), single precision vertex positions, packed attributes (2x uvec4)
- Textured Phong and double precision matrix performance - double precision per-triangle matrix in buffer (dmat4),
  double precision per-scene view matrix in uniform (dmat4), both matrices multiplied in double precision, single precision
  vertex positions, single precision per-scene perspective matrix in uniform (mat4), packed attributes (2x uvec4)
- Textured Phong and double precision matrix performance - double precision per-triangle matrix in buffer (dmat4),
  double precision per-scene view matrix in uniform (dmat4), both matrices multiplied in double precision, double precision
  vertex positions (dvec3), single precision per-scene perspective matrix in uniform (mat4), packed attributes (3x uvec4)
- Textured Phong and double precision matrix performance using GS - double precision per-triangle matrix in buffer (dmat4),
  double precision per-scene view matrix in uniform (dmat4), both matrices multiplied in double precision, double precision
  vertex positions (dvec3), single precision per-scene perspective matrix in uniform (mat4), packed attributes (3x uvec4)


## Shared vertex performance

The test measures rendering performance of triangle strip-like geometry.
The pipeline is using VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST.
For each new triangle, we duplicate the coordinates of the two vertices of the previous triangle.
A number of tests are performed, using different lenght of the strip for each test.
The vkCmdDraw() call is used for each strip.

The simplified rendering code is as follows:
```c++
for(uint32_t i=0,e=3*numberOfTriangles; i<e; i+=trianglesPerStrip*3)
	vkCmdDraw(
		commandBuffer,
		trianglesPerStrip*3,  // vertexCount
		1,  // instanceCount
		i,  // firstVertex
		0  // firstInstance
	);
```

The rest of the code is based on [Textured Phong and PAT](#textured-phong-and-pat-performance).
So, it contains VS that implements textured Phong, per-scene PAT v2 (Position-Attitude-Transform:
translation (vec3)+quaternion rotation (vec4)) that is read from the same index in the buffer,
2x uniform matrix (mat4+mat4) and 2x packed attribute. 

The positions of vertices are generated in a way to specify tiny triangles
in between pixel sampling locations distributed roughly across the whole framebuffer.
So, they do not produce any fragments in the rasterizer.


## Indexed shared vertex performance

The test measures rendering performance of indexed triangle strip-like geometry.
The pipeline is using VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST.
For each new triangle, we reuse two indices of the previous triangle.
A number of tests are performed, using different lenght of the strip for each test.
The vkCmdDrawIndexed() call is used for each strip.

The simplified rendering code is as follows:
```c++
for(uint32_t i=0,e=3*numberOfTriangles; i<e; i+=trianglesPerStrip*3)
	vkCmdDrawIndexed(
		commandBuffer,
		trianglesPerStrip*3,  // indexCount
		1,  // instanceCount
		i,  // firstIndex
		0,  // vertexOffset
		0  // firstInstance
	);
```

The rest of the code is similar to the [previous test](#shared-vertex-performance).


## Triangle strip performance

The test measures rendering performance of triangle strips.
The pipeline uses VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP. So, two vertices of each
triangle are expected to be reused from the previous triangle by Vulkan device.
A number of tests are performed, using different lenght of the strip for each test.
The vkCmdDraw() call is used for each strip.

The simplified rendering code is as follows:
```c++
for(uint32_t i=0,e=totalNumberOfIndices; i<e; i+=2+maxTrianglesPerStrip)
	for(uint32_t j=i,je=i+maxTrianglesPerStrip; j<je; j+=trianglesPerStrip)
		vkCmdDraw(
			commandBuffer,
			trianglesPerStrip+2,  // vertexCount
			1,  // instanceCount
			j,  // firstVertex
			0  // firstInstance
		);
```

The rest of the code is similar to the [previous two tests](#shared-vertex-performance).


## Indexed triangle strip performance

The test measures rendering performance of indexed triangle strips.
The pipeline uses VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP. So, two vertices of each
triangle are expected to be reused from the previous triangle by Vulkan device.
The indices in index buffer increase by one on each next position.
A number of tests are performed, using different lenght of the strip for each test.
The vkCmdDrawIndexed() call is used for each strip.

The simplified rendering code is as follows:
```c++
for(uint32_t i=0,e=totalNumberOfIndices; i<e; i+=2+maxTrianglesPerStrip)
	for(uint32_t j=i,je=i+maxTrianglesPerStrip; j<je; j+=trianglesPerStrip)
		vkCmdDrawIndexed(
			commandBuffer,
			trianglesPerStrip+2,  // indexCount
			1,  // instanceCount
			j,  // firstIndex
			0,  // vertexOffset
			0  // firstInstance
		);
```

The rest of the code is similar to the [previous three tests](#shared-vertex-performance).


## Primitive restart performance

The primitive restart tests should answer, among others, question of primitive restart overhead.

The tests based on [Textured Phong and PAT tests](#textured-phong-and-pat-performance).
Thus, VS is using packed attributes (2x vec4) extracted into position+normal+color+textureCoordinates,
two uniform matrices (mat4+mat4) for perspective and view matrix, and
constant single PAT v2 (vec3+vec4) read from the same index in the buffer (Position-Attitude-Transform,
e.g. translation by vec3 and rotation by vec4 quaternion). No framents are produced in these tests.

### Primitive restart performance with single per-scene vkCmdDrawIndexed() call

Five tests are performed:
- for having -1 after each triangle (three indices followed by -1),
- after each two triangles (four indices followed by -1),
- after each five triangles (seven indices followed by -1),
- after each eight triangles (ten indices followed by -1),
- after each thousand triangles (1002 indices followed by -1).

The simplified rendering code is as follows:
```c++
vkCmdDrawIndexed(
	commandBuffer,
	numIndicesInTheScene,  // indexCount
	1,  // instanceCount
	0,  // firstIndex
	0,  // vertexOffset
	0  // firstInstance
);
```

More details are in the [Primitive restart section](#primitive-restart-performance).

### Primitive restart performance with per-strip vkCmdDrawIndexed() call

The test is the same as the [previous one](#primitive-restart-performance-with-single-per-scene-vkcmddrawindexed-call)
except it uses vkCmdDrawIndexed for each strip:

```c++
for(uint32_t i=0,e=numStrips*numIndicesPerStrip; i<e; i+=numIndicesPerStrip)
	cb.drawIndexed(numIndicesPerStrip, 1, i, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
	vkCmdDrawIndexed(
		commandBuffer,
		numIndicesPerStrip,  // indexCount
		1,  // instanceCount
		i,  // firstIndex
		0,  // vertexOffset
		0  // firstInstance
	);
```

### Primitive restart using 2x -1 followed by indexed triangle

Each triangle is followed by two -1. It uses single per-scene vkCmdDrawIndexed() call.

### Primitive restart using 5x -1 followed by indexed triangle

Each triangle is followed by five -1. It uses single per-scene vkCmdDrawIndexed() call.

### Primitive restart -1 performance, each triangle indices replaced by one -1

The test measures processing performance of -1.
Each triangle is replaced by single -1. It uses single per-scene vkCmdDrawIndexed() call.


## The same vertex processing performance

The five tests are processing the same vertex or vertex index.
What differs is the use of primitive restart, using of indices,
and the use of triangle strip or triangle list.

All the tests are using textured Phong vertex shader, constant single PAT v2 (vec3+vec4)
read from the same index in the buffer, 2x uniform matrix (mat4+mat4) as
view and perspective matrix, and packed attributes (2x uvec4).
Geometry is provided in a way to not produce any fragments in the rasterizer.

### All 1 index performance, the pipeline uses primitive restart

The test uses the value 1 in the index buffer for all vertices in the scene.
It is rendered by single vkCmdDrawIndexed() call while using pipeline with primitive restart
and one per-scene triangle strip (composed of indices with value 1).

### All 1 index performance, the pipeline uses single per-scene triangle strip

The test uses the value 1 in the index buffer for all vertices in the scene.
It is rendered by single vkCmdDrawIndexed() call
and one per-scene triangle strip (composed of indices with value 1).

### All 1 index performance, the pipeline uses indexed triangle list

The test uses the value 1 in the index buffer for all vertices in the scene.
It is rendered by single vkCmdDrawIndexed() call as triangle list
(composed of indices with value 1).

### The same vertex triangle strip performance

The test uses vertices with the same coordinates and attributes for the whole scene.
All the vertices are rendered by single vkCmdDraw() call while using triangle strip.

### The same vertex triangle list performance

The test uses vertices with the same coordinates and attributes for the whole scene.
All the vertices are rendered by single vkCmdDraw() call while using triangle list.

