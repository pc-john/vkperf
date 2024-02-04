# vkperf tests

Vulkan Performance Measurement Tool perform various tests to measure performance characteristics of particular Vulkan device.
The tests are described in detail in this file.

The list of tests follows:
1. [Triangle list](#triangle-list)
2. [Indexed triangle list](#indexed-triangle-list)
3. [Indexed triangle list that reuses two indices of the previous triangle](#indexed-triangle-list-that-reuses-two-indices-of-the-previous-triangle)
4. [Triangle strips of various lengths](#triangle-strips-of-various-lengths)
5. [Indexed triangle strips of various lengths](#indexed-triangle-strips-of-various-lengths)
6. [Primitive restart indexed triangle strips of various lengths](#primitive-restart-indexed-triangle-strips-of-various-lengths)
7. [Primitive restart, each triangle is replaced by one -1](#primitive-restart-each-triangle-is-replaced-by-minus-one)
8. [Primitive restart, only zeros in the index buffer](#primitive-restart-only-zeros-in-the-index-buffer)
9. [Instancing throughput of vkCmdDraw()](#instancing-throughput-of-vkcmddraw)
10. [Instancing throughput of vkCmdDrawIndexed()](#instancing-throughput-of-vkcmddrawindexed)
11. [Instancing throughput of vkCmdDrawIndirect()](#instancing-throughput-of-vkcmddrawindirect)
12. [Instancing throughput of vkCmdDrawIndexedIndirect()](#instancing-throughput-of-vkcmddrawindexedindirect)
13. [vkCmdDraw() throughput](#vkcmddraw-throughput)
14. [vkCmdDrawIndexed() throughput](#vkcmddrawindexed-throughput)
15. [VkDrawIndirectCommand processing throughput](#vkdrawindirectcommand-processing-throughput)
16. [VkDrawIndirectCommand processing throughput with stride 32](#vkdrawindirectcommand-processing-throughput-with-stride)
17. [VkDrawIndexedIndirectCommand processing throughput](#vkdrawindexedindirectcommand-processing-throughput)
18. [VkDrawIndexedIndirectCommand processing throughput with stride 32](#vkdrawindexedindirectcommand-processing-throughput-with-stride)
19. [VS throughput using vkCmdDraw()](#vs-throughput-using-vkcmddraw)
20. [VS throughput using vkCmdDrawIndexed()](#vs-thoughput-using-vkcmddrawindexed)
21. [VS producing output position from VertexIndex and InstanceIndex using vkCmdDraw()](#vs-producing-output-position-from-vertexindex-and-instanceindex-using-vkcmddraw)
22. [VS producing output position from VertexIndex and InstanceIndex using vkCmdDrawIndexed()](#vs-producing-output-position-from-vertexindex-and-instanceindex-vkcmddrawindexed)
23. [GS one triangle in and no triangle out](#gs-one-triangle-in-and-no-triangle-out)
24. [GS one triangle in and single constant triangle out](#gs-one-triangle-in-and-single-constant-triangle-out)
25. [GS one triangle in and two constant triangles out](#gs-one-triangle-in-and-two-constant-triangles-out)
26. ... 44. [Attributes and buffers](#attributes-and-buffers)
- [Attribute tests](#attribute-tests)
- [Buffer tests](#buffer-tests)
- [Interleaved attribute tests](#interleaved-attribute-tests)
- [Interleaved buffer tests](#interleaved-buffer-tests)
- [Packed data tests](#packed-data-tests)
- [Attribute conversion test](#attribute-conversion-test)
45. ... 59. [Transformations](#transformations)
- [Uniform vs buffer vs attribute matrix tests](#uniform-vs-buffer-vs-attribute-matrix-tests)
- [Single whole scene matrix test](#single-whole-scene-matrix-test)
- [Single per-triangle matrix tests](#single-per-triangle-matrix-tests)
- [Three matrices test](#three-matrices-test)
- [Five matrices tests](#five-matrices-tests)
60. ... 77. [Textured Phong performance](#textured-phong-performance)
- [Textured Phong, matrices and four attributes](#textured-phong-matrices-and-four-attributes)
- [Textured Phong, matrices and packed attributes](#textured-phong-matrices-and-packed-attributes)
- [Textured Phong and PAT performance](#textured-phong-and-pat-performance)
- [Textured Phong, PAT, indexed rendering and primitive restart](#textured-phong-pat-indexed-rendering-and-primitive-restart)
- [Textured Phong and double precision matrix performance](#textured-phong-and-double-precision-matrix-performance)
78. ... 88. [Fragment throughput](#fragment-throughput)
89. [Transfer of consecutive memory blocks](#transfer-of-consecutive-memory-blocks)
90. [Transfer of spaced memory blocks](#transfer-of-spaced-memory-blocks)


## Triangle list

Triangle list test measures number of triangles per second that particular Vulkan device can render.
The test uses triangle list primitive topology and per-scene glCmdDraw().

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


## Indexed triangle list

Indexed triangle list test measures number of rendered triangles per second.

The test is the same as the [previous test](#triangle-list) except
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

No indices are shared between the triangles, e.g. no vertex reuse.


## Indexed triangle list that reuses two indices of the previous triangle

The test is the same as the [previous test](#indexed-triangle-list) except
that each triangle reuses two indices of the previous triangle.


## Triangle strips of various lengths

The test measures triangle throughput when rendering triangle strips of various lengths.

The test uses triangle strip primitive topology and per-strip vkCmdDraw() call:

```c++
vkCmdDraw(
	commandBuffer,
	numberOfStripTriangles+2,  // vertexCount
	1,  // instanceCount
	stripStartIndex,  // firstVertex
	0   // firstInstance
);
```

## Indexed triangle strips of various lengths

The test measures triangle throughput when rendering triangle strips of various lengths.

The test uses triangle strip primitive topology and per-strip vkCmdDrawIndexed() call:

```c++
vkCmdDrawIndexed(
	commandBuffer,
	numberOfStripTriangles+2,  // indexCount
	1,  // instanceCount
	stripStartIndex,  // firstIndex
	0,  // vertexOffset
	0   // firstInstance
);
```


## Primitive restart indexed triangle strips of various lengths

The test measures triangle throughput when using primitive restart.
Number of tests is performed to measure the throughput when strips of various lengths are stored in the index buffer.

The test uses per-scene vkCmdDrawIndexed() call:

```c++
vkCmdDrawIndexed(
	commandBuffer,
	numIndicesPerStrip*numStrips,  // indexCount
	1,  // instanceCount
	0,  // firstIndex
	0,  // vertexOffset
	0   // firstInstance
);
```


## Primitive restart, each triangle is replaced by one -1

The test measures -1 overhead when using primitive restart.
Each triangle is replaced by single -1. The scene is rendered by the single vkCmdDrawIndexed() call:

```c++
vkCmdDrawIndexed(
	commandBuffer,
	numMinusOneValues,  // indexCount
	1,  // instanceCount
	0,  // firstIndex
	0,  // vertexOffset
	0   // firstInstance
);
```


## Primitive restart, only zeros in the index buffer

The test measures triangle throughput when index buffer contains only zeros, e.g. degenerated triangles.


## Instancing throughput of vkCmdDraw()

The test measures instancing performance, e.g. number of rendered instances per second
that particular Vulkan device can process.

The test renders single triangle instanced very many times.
This way, the triangle throughput is equal to instance throughput.

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

The test measures instancing throughput when using indexed draw call:

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

The test measures instancing performance of vkCmdDrawIndirect().

VkDrawIndirectCommand structure instances single triangle very many times.
The struct is initialized with the following content:

```c++
indirectBufferPtr->vertexCount = 3;
indirectBufferPtr->instanceCount = numberOfTriangles;
indirectBufferPtr->firstVertex = 0;
indirectBufferPtr->firstInstance = 0;
```

The VkDrawIndirectCommand structure is rendered by a single vkCmdDrawIndirect() call:

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

The test measures instancing throughput of vkCmdDrawIndexedIndirect().

VkDrawIndexedIndirectCommand structure instances single triangle very many times.
The struct is initialized with the following content:

```c++
	indirectIndexedBufferPtr->indexCount = 3;
	indirectIndexedBufferPtr->instanceCount = numTriangles;
	indirectIndexedBufferPtr->firstIndex = 0;
	indirectIndexedBufferPtr->vertexOffset = 0;
	indirectIndexedBufferPtr->firstInstance = 0;
```

The VkDrawIndexedIndirectCommand structure is processed by a single vkCmdDrawIndexedIndirect() call:

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


## vkCmdDraw() throughput

The test measures throughput of vkCmdDraw().
Each vkCmdDraw() renders single triangle.
Thus, triangle throughput is equal to vkCmdDraw() throughput.

Draw code is equal to the following one:

```c++
for(uint32_t i=0; i<numberOfTriangles; i++)
	vkCmdDraw(
		commandBuffer,
		3,  // vertexCount
		1,  // instanceCount
		i*3,  // firstVertex
		0   // firstInstance
	);
```

The vertex shader outputs constant coordinates, thus producing zero-sized triangles:

```c++
void main() {
	gl_Position = vec4(0, 0, 0.5, 1);
}
```


## vkCmdDrawIndexed() throughput

The test measures throughput of vkCmdDrawIndexed().
Each vkCmdDrawIndexed() renders single triangle.
Thus, triangle throughput is equal to vkCmdDrawIndexed() throughput.

Draw code is equal to the following one:

```c++
for(uint32_t i=0; i<numberOfTriangles; i++)
	vkCmdDrawIndexed(
		commandBuffer,
		3,  // indexCount
		1,  // instanceCount
		i*3,  // firstIndex
		0,  // vertexOffset
		0   // firstInstance
	);
```


## VkDrawIndirectCommand processing throughput

The test measures the number of VkDrawIndirectCommand structures processed per second.
Each VkDrawIndirectCommand renders single triangle,
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

The rendering code is equal to the following one:

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

The test measures the number of VkDrawIndexedIndirectCommand structures processed per second.
Each VkDrawIndexedIndirectCommand renders single triangle,
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

The rendering code is equal to the following one:

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


## VS throughput using vkCmdDraw()

The test measures number of vertices per second that particular Vulkan device can process.
The test uses per-scene glCmdDraw() and minimal VS that just writes constant output position.
Thus, zero size triangles are produced.

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


## VS throughput using vkCmdDrawIndexed()

The test measures number of vertices per second that particular Vulkan device can process.
The test uses per-scene glCmdDrawIndexed() and minimal VS that just writes constant output position.
Thus, zero size triangles are produced.

The test is the same as the [previous test](#vs-throughput-using-vkcmddraw) except
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

No indices are shared between the triangles, e.g. no vertex reuse.


## VS producing output position from VertexIndex and InstanceIndex using vkCmdDraw()

The test measures vertex shader throughput with gl_VertexIndex and gl_InstanceIndex overhead:

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


## VS producing output position from VertexIndex and InstanceIndex using vkCmdDrawIndexed()

The test is the same as the [previous test](#vs-producing-output-position-from-vertexindex-and-instanceindex-using-vkcmddraw)
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


## GS one triangle in and no triangle out

The test measures geometry shader throughput. It receives one triangle as its input and produces no output:

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


## GS one triangle in and single constant triangle out

The test measures geometry shader throughput. It receives one triangle as its input and
produces one single constant triangle as its output:

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
as in the [previous test](#gs-one-triangle-in-and-no-triangle-out).


## GS one triangle in and two constant triangles out

The test measures geometry shader throughput. It receives one triangle as its input and
produces two constant triangles as its output:

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
as in the [previous two tests](#gs-one-triangle-in-and-no-triangle-out).


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
No vertices are shared between triangles.
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


## Fragment throughput

vkperf uses full-screen or full-framebuffer quads to render fragments. By default, framebuffer dimension is 1920x1080, e.g. FullHD resolution.
The framebuffer is composed of color and depth buffer and does not use multisampling. The color buffer uses R8G8B8A8Srgb format.
The depth format uses the first available format from the following list: D32Sfloat, D32SfloatS8Uint and D24UnormS8Uint.
The depth compare operation is set to Less and depth writes are turned on.

### Single full-framebuffer quad, constant color FS

The test renders single full-framebuffer quad while measuring fragment throughput.
Fragment shader writes single constant value:

```c++
void main() {
	fragColor = vec4(1, 1, 0, 1);
}
```

### 10x full-framebuffer quad, constant color FS

The test renders ten full-framebuffer quads. Quads are rendered with decreasing z distance as produced by VS:

```c++
const vec2 vertices2D[] = {
	{ -1.,-1. },
	{ -1.,+1. },
	{ +1.,-1. },
	{ +1.,+1. },
};

void main() {
	gl_Position = vec4(vertices2D[gl_VertexIndex], 0.6-(gl_InstanceIndex*0.05), 1.);
}
```

The following call is used to render the scene:

```c++
vkCmdDraw(
	commandBuffer,
	4,  // vertexCount
	numFullscreenQuads,  // instanceCount
	0,  // firstVertex
	0   // firstInstance
);
```

### Four smooth interpolators (4x vec4), 10x fullscreen quad

The test measures the performance impact of smooth interpolators.
It renders ten full-framebuffer quads, from the farthest to the closest
while interpolating four vec4 variables:

```c++
layout(location=0) in smooth vec4 v1;
layout(location=1) in smooth vec4 v2;
layout(location=2) in smooth vec4 v3;
layout(location=3) in smooth vec4 v4;
layout(location=0) out vec4 fragColor;

void main() {
	fragColor = vec4(dot(v1,v3), dot(v2,v4), 1., 1.);  // do something with v1..v4 to avoid them being optimized out
}
```

### Four flat interpolators (4x vec4), 10x fullscreen quad

The test measures the performance impact of flat interpolators.
It renders ten full-framebuffer quads, from the farthest to the closest
while having four vec4 flat input variables in the fragment shader:

```c++
layout(location=0) in flat vec4 v1;
layout(location=1) in flat vec4 v2;
layout(location=2) in flat vec4 v3;
layout(location=3) in flat vec4 v4;
layout(location=0) out vec4 fragColor;

void main() {
	fragColor = vec4(dot(v1,v3), dot(v2,v4), 1., 1.);  // do something with v1..v4 to avoid them being optimized out
}
```

### Four textured phong interpolators (vec3+vec3+vec4+vec2), 10x fullscreen quad:                     ",

The test measures the performance impact of interpolators used usually for textured phong lighting
(vec3 position, vec3 normal, vec4 color, vec2 texCoords).
It renders ten full-framebuffer quads, from the farthest to the closest
while interpolating inputs of the fragment shader:

```c++
layout(location=0) in smooth vec3 eyePosition;
layout(location=1) in smooth vec3 eyeNormal;
layout(location=2) in smooth vec4 color;
layout(location=3) in smooth vec2 texCoord;
layout(location=0) out vec4 fragColor;

void main() {
	fragColor = vec4(dot(vec4(eyePosition,1.), color), dot(eyeNormal, vec3(texCoord,1.)), 0., 1.);  // do something with the inputs to avoid them being optimized out
}
```

### Textured Phong, packed uniforms

The test performs full Phong computation in fragment shader. It is composed of diffuse reflection, specular effect, light attenuation (quadratic, linear, constant),
ambient and global ambient light contributions, material emission, and texture application. The texture used is single texel texture (1x1, R8G8B8A8) sampled with trilinear sampler.
Material and light data are packed and stored in uniforms. They take 132 bytes in total.

Ten full-framebuffer quads are rendered, from the farthest to the closest.

The fragment shader code follows:

```c++
layout(location=0) in smooth vec3 eyePosition;
layout(location=1) in smooth vec3 eyeNormal;
layout(location=2) in smooth vec4 color;
layout(location=3) in smooth vec2 texCoord;

layout(binding=0,std140) uniform Material {
	vec4 materialData[3]; // ambient on offset 0, diffuse on offset 12, specular on offset 24, emissive on offset 36
	layout(offset=48) float shininess;
	layout(offset=52) int textureMode; // 0 - no texturing, 0x2100 - modulate, 0x1e01 - replace, 0x2101 - decal*/
};
layout(binding=1) uniform Global {
	vec3 globalAmbientLight;
};
layout(binding=2) uniform Light {
	vec4 lightPosition;
	vec4 lightData[3]; // attenuation on offset 16, ambient on offset 28, diffuse on offset 40, specular on offset 52
};
layout(binding=3) uniform sampler2D textureSampler;

layout(location=0) out vec4 fragColor;


void main() {

	// read texture
	vec4 textureColor;
	if(textureMode != 0)
		textureColor = texture(textureSampler, texCoord);

	// surface color
	if(textureMode == 0x1e01)  // if GL_REPLACE

		// apply GL_REPLACE texture
		fragColor = vec4(textureColor.rgb, textureColor.a*color.a);

	else {

		// l - vertex to light direction, in eye coordinates
		vec3 l = lightPosition.xyz;
		if(lightPosition.w != 0.) {
			if(lightPosition.w != 1.)
				l /= lightPosition.w;
			l -= eyePosition;
		}
		float lLen = length(l);
		l /= lLen;

		// attenuation
		float attenuation = 1.;
		if(lightPosition.w != 0.)
			attenuation /= lightData[0].x + lightData[0].y*lLen + lightData[0].z*lLen*lLen;

		// n - Normal of vertex, in eye coordinates
		vec3 n = normalize(eyeNormal);

		// invert normals on back facing triangles
		if(gl_FrontFacing == false)
			n = -n;

		// unpack material data
		vec3 ambientColor = vec3(materialData[0].rgb);
		vec3 diffuseColor = vec3(materialData[0].a, materialData[1].rg);
		vec3 specularColor = vec3(materialData[1].ba, materialData[2].r);
		vec3 emissiveColor = vec3(materialData[2].gba);

		// unpack light data
		vec3 ambientLight = vec3(lightData[0].a, lightData[1].rg);
		vec3 diffuseLight = vec3(lightData[1].ba, lightData[2].r);
		vec3 specularLight = vec3(lightData[2].gba);

		// Lambert's diffuse reflection
		float nDotL = dot(n, l);
		fragColor.rgb = (ambientLight * ambientColor +
		                 diffuseLight * diffuseColor * max(nDotL, 0.)) * attenuation;
		fragColor.a = color.a;

		// global ambient and emissive color
		fragColor.rgb += globalAmbientLight * ambientColor + emissiveColor;

		// apply texture
		if(textureMode != 0) {

			if(textureMode == 0x2100) // GL_MODULATE
				fragColor = fragColor * textureColor;
			else if(textureMode == 0x2101)  // GL_DECAL
				fragColor = vec4(fragColor.rgb * (1-textureColor.a) + textureColor.rgb * textureColor.a, fragColor.a);

		}

		// specular effect (it is applied after the texture)
		if(nDotL > 0.) {
			vec3 r = reflect(-l, n);
			float rDotV = dot(r, -normalize(eyePosition));
			if(rDotV > 0.)
				fragColor.rgb += specularLight * specularColor * pow(rDotV, shininess) * attenuation;
		}

	}

}
```


### Textured Phong, not packed uniforms

The test is the same as [the previous one](#textured-phong-packed-uniforms) except that
it does not pack material and light data that are stored in uniforms.
The material and light data takes 164 bytes (compared to 132 bytes when the data are packed).

The uniforms looks like:

```c++
layout(binding=0,std140) uniform Material {
	vec4 ambientColor;
	vec4 diffuseColor;
	vec4 specularColor;
	vec4 emissiveColor;
	float shininess;
	int textureMode; // 0 - no texturing, 0x2100 - modulate, 0x1e01 - replace, 0x2101 - decal*/
};
layout(binding=1) uniform Global {
	vec3 globalAmbientLight;
};
layout(binding=2) uniform Light {
	vec4 lightPosition;
	vec4 lightAttenuation;
	vec4 ambientLight;
	vec4 diffuseLight;
	vec4 specularLight;
};
layout(binding=3) uniform sampler2D textureSampler;
```


### Simplified Phong, no texture, no specular

The test uses simplified Phong without specular effect and without texturing.
It renders ten full-framebuffer quads, from the farthest to the closest.

The fragment shader code follows:

```c++
layout(location=0) in smooth vec3 eyePosition;
layout(location=1) in smooth vec3 eyeNormal;

layout(binding=0,std140) uniform Material {
	vec4 ambientColor;  // alpha not used
	vec4 diffuseColor;  // alpha used for transparency
};
layout(binding=1) uniform Global {
	vec3 globalAmbientLight;
};
layout(binding=2) uniform Light {
	vec4 lightData[3]; // position on offset 0, attenuation on offset 12, ambient on offset 24, diffuse on offset 36
};

layout(location=0) out vec4 fragColor;


void main() {

	// l - vertex to light direction, in eye coordinates
	vec3 l = lightData[0].xyz - eyePosition;
	float lLen = length(l);
	l /= lLen;

	// attenuation
	float attenuation = 1. / (lightData[0].w + lightData[1].x*lLen + lightData[1].y*lLen*lLen);

	// n - Normal of vertex, in eye coordinates
	vec3 n = normalize(eyeNormal);

	// invert normals on back facing triangles
	if(gl_FrontFacing == false)
		n = -n;

	// unpack data
	vec3 ambientLight = vec3(lightData[1].zw, lightData[2].x);
	vec3 diffuseLight = lightData[2].yzw;

	// Lambert's diffuse reflection
	float nDotL = dot(n,l);
	fragColor.rgb = (ambientLight * ambientColor.rgb +
	                 diffuseLight * diffuseColor.rgb * max(nDotL, 0.)) * attenuation;
	fragColor.a = diffuseColor.a;

	// global ambient
	fragColor.rgb += globalAmbientLight * ambientColor.rgb;

}
```


### Simplified Phong, no texture, no specular, one uniform for material and light data

The test is the same as [the previous one](#simplified-phong-no-texture-no-specular) except that
it uses only one uniform with all material and light data:

```c++
layout(binding=0,std140) uniform AllInOneUniform {
	vec4 ambientColor;  // alpha not used
	vec4 diffuseColor;  // alpha used for transparency
	vec4 globalAmbientLight; // only rgb values used
	vec4 lightData[3]; // position on offset 0, attenuation on offset 12, ambient on offset 24, diffuse on offset 36
};
```


### Constant color from uniform, 1x uniform (vec4) in FS

The test just assigns color given by vec4 uniform to the output fragment.
It renders ten full-framebuffer quads, from the farthest to the closest.

The FS looks like:

```c++
layout(binding=0,std140) uniform Color {
	vec4 color;
};

void main()
{
	fragColor = color;
}
```


### Constant color from uniform, 1x uniform (uint) in FS

The test is the same as [the previous one](#constant-color-from-uniform-1x-uniform-vec4-in-fs) except that
it uses uint uniform for the color:

```c++
layout(binding=0) uniform Color {
	uint color;
};

void main()
{
	fragColor = unpackUnorm4x8(color);
}
```


## Transfer of consecutive memory blocks

The transfer tests measure the copy throughput between host and device memory.
The transfers use blocks of various sizes. For very small blocks, transfer overhead is usually apparent.
For very large blocks, the throughput limit often shows itself.

```c++
for(size_t offset = 0; offset < transferSize*numTransfers; offset += transferSize)
{
	vkCmdCopyBuffer(
		commandBuffer,
		srcBuffer,
		dstBuffer,
		1,  // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(  // pRegions
			offset,  // srcOffset
			offset,  // dstOffset
			transferSize  // size
		)
	);
}
```


## Transfer of spaced memory blocks

The transfer test of spaced memory blocks tries to avoid possible optimization of concatenating of consecutive transfers.
So, not tranfered memory blocks are put inbetween transfered memory blocks

The transfer code follows:

```c++
for(size_t offset = 0; offset < numTransfers*transferSize*2; offset += transferSize*2)
{
	vkCmdCopyBuffer(
		commandBuffer,
		srcBuffer,
		dstBuffer,
		1,  // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(  // pRegions
			offset,  // srcOffset
			offset,  // dstOffset
			transferSize  // size
		)
	);
}
```
