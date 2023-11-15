# vkperf tests

Vulkan Performance Measurement Tool perform various tests to measure performance characteristics of particular Vulkan device.
The tests are described in detail in this file.

The list of tests follows:
1. [VS max throughput](#vs-max-throughput)
2. [VS VertexIndex and InstanceIndex forming output](#vs-vertexindex-and-instanceindex-forming-output)
3. [GS max throughput when no output is produced](#gs-max-throughput-when-no-output-is-produced)
4. [GS max throughput when single constant triangle is produced](#gs-max-throughput-when-single-constant-triangle-is-produced)
5. [GS max throughput when two constant triangles are produced](#gs-max-throughput-when-two-constant-triangles-are-produced)
6. [Instancing throughput of vkCmdDraw()](#instancing-throughput-of-vkcmddraw)
7. [Instancing throughput of vkCmdDrawIndirect()](#instancing-throughput-of-vkcmddrawindirect)
8. [Draw command throughput](#draw-command-throughput)
9. [Draw command throughput with vec4 attribute](#draw-command-throughput-with-vec4-attribute)
10. [Indirect command processing throughput](#indirect-command-processing-throughput)
11. [Indirect command processing throughput with vec4 attribute](#indirect-command-processing-throughput-with-vec4-attribute)
12. ... 31. [Attribute and buffer performance](#attribute-and-buffer-performance)
- [Attribute tests](#attribute-tests)
- [Buffer tests](#buffer-tests)
- [Interleaved attribute tests](#interleaved-attribute-tests)
- [Interleaved buffer tests](#interleaved-buffer-tests)
- [Packed data tests](#packed-data-tests)
- [Attribute conversion test](#attribute-conversion-test)


## VS max throughput

Vertex shader maximum throughput test measures number of triangles per second that particular Vulkan device can render.
To get vertex throughput, multiply the result by 3.

The test uses simple vertex shader with constant output. Thus, zero size triangle is produced.
The shader's main() code is as follows:

```c++
void main() {
	gl_Position = vec4(0, 0, 0.5, 1);
}
```

Rendering is performed using single draw call for the whole scene:

```c++
vkCmdDraw(
	commandBuffer,
	3*numberOfTriangles,  // vertexCount
	1,  // instanceCount
	0,  // firstVertex
	0   // firstInstance
);
```


## VS VertexIndex and InstanceIndex forming output

Vertex shader maximum throughput test uses simple shader that utilizes gl_VertexIndex and gl_InstanceIndex input variables.
No attributes, buffers, descriptor sets, push constants or specialization constants are used:

```c++
void main() {
	gl_Position = vec4(0, 0, float(gl_VertexIndex + gl_InstanceIndex) * 1e-20, 1);
}
```

Rendering is performed using single draw call for the whole scene:

```c++
vkCmdDraw(
	commandBuffer,
	3*numberOfTriangles,  // vertexCount
	1,  // instanceCount
	0,  // firstVertex
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

Rendering is performed using single draw call for the whole scene:

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

Geometry shader maximum throughput test uses the following geometry shader to produce single constant triangle:

```c++
layout(triangles) in;
layout(triangle_strip,max_vertices=3) out;
void main() {
	gl_Position=vec4(0,0,0.5,1);
	EmitVertex();
	gl_Position=vec4(0,0,0.6,1);
	EmitVertex();
	gl_Position=vec4(0,1e-10,0.4,1);
	EmitVertex();
}
```

It is fed by empty vertex shader:

```c++
void main() {
}
```

Rendering is performed using single draw call for the whole scene:

```c++
vkCmdDraw(
	commandBuffer,
	3*numberOfTriangles,  // vertexCount
	1,  // instanceCount
	0,  // firstVertex
	0   // firstInstance
);
```


## GS max throughput when two constant triangles are produced

Geometry shader maximum throughput test uses the following geometry shader to produce two constant triangles:

```c++
layout(triangles) in;
layout(triangle_strip,max_vertices=6) out;
void main() {
	gl_Position=vec4(0,0,0.5,1);
	EmitVertex();
	gl_Position=vec4(0,0,0.6,1);
	EmitVertex();
	gl_Position=vec4(0,1e-10,0.4,1);
	EmitVertex();
	EndPrimitive();
	gl_Position=vec4(0,0,0.7,1);
	EmitVertex();
	gl_Position=vec4(0,0,0.8,1);
	EmitVertex();
	gl_Position=vec4(0,1e-10,0.9,1);
	EmitVertex();
}
```

The geometry shader is fed by empty vertex shader:

```c++
void main() {
}
```

Rendering is performed using single draw call for the whole scene:

```c++
vkCmdDraw(
	commandBuffer,
	3*numberOfTriangles,  // vertexCount
	1,  // instanceCount
	0,  // firstVertex
	0   // firstInstance
);
```


## Instancing throughput of vkCmdDraw()

Instancing throughput test measures instancing performance, e.g. number of instances per second
that particular Vulkan device can render.

The test uses single triangle instanced very many times. Thus, the measured number
of rendered triangles is equal to the number of rendered instances. 

The whole scene is rendered using single vkCmdDraw() call:

```c++
vkCmdDraw(
	commandBuffer,
	3,  // vertexCount
	numberOfTriangles,  // instanceCount
	0,  // firstVertex
	0   // firstInstance
);  // vertexCount, instanceCount, firstVertex, firstInstance
```

The vertex shader in outputs constant coordinates, thus producing zero-sized triangles:

```c++
void main() {
	gl_Position = vec4(0, 0, 0.5, 1);
}
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

The VkDrawIndirectCommand structure is processed by vkCmdDrawIndirect() call:

```c++
vkCmdDrawIndirect(
	indirectBuffer,  // buffer
	0,  // offset
	1,  // drawCount
	sizeof(VkDrawIndirectCommand)  // stride
);
```

The vertex shader in use outputs constant coordinates, thus producing zero-sized triangles:

```c++
void main() {
	gl_Position = vec4(0, 0, 0.5, 1);
}
```


## Draw command throughput

Draw command throughput test measures performance of vkCmdDraw().
Each draw call renders single triangle. Thus, triangle rendering performance is equal
to the draw call performance in this test.

Draw code is equal to the following one:

```c++
for(uint32_t i=0; i<numberOfTriangles; i++)
	vkCmdDraw(
		commandBuffer,
		3,  // vertexCount
		1,  // instanceCount
		3*numberOfTriangles,  // firstVertex
		0   // firstInstance
	);
```

The vertex shader in use outputs constant coordinates, thus producing zero-sized triangles:

```c++
void main() {
	gl_Position = vec4(0, 0, 0.5, 1);
}
```


## Draw command throughput with vec4 attribute

Draw command throughput test with vec4 attribute is equal to [the previous test](#draw-command-throughput)
except that it uses position attribute in vertex shader:

```c++
layout(location=0) in vec4 inPosition;
void main() {
	gl_Position = inPosition;
}
```

Vec4 coordinates are provided in a way to not produce any fragments in the rasterizer.
They are tiny triangles in between pixel sampling locations distributed roughly across the whole screen.


## Indirect command processing throughput

Indirect command processing throughput test measures the number of VkDrawIndirectCommand
processed per second. Each VkDrawIndirectCommand contains single triangle,
thus number of processed triangles is equal to the number of processed
VkDrawIndirectCommand structures. The content of VkDrawIndirectCommand is as follows:

```c++
indirectBufferPtr[i].vertexCount = 3;
indirectBufferPtr[i].instanceCount = 1;
indirectBufferPtr[i].firstVertex = i * 3;
indirectBufferPtr[i].firstInstance = 0;
```

The vertex shader in use outputs constant coordinates, thus producing zero-sized triangles:

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


## Indirect command processing throughput with vec4 attribute

Indirect command processing throughput test with vec4 attribute is equal to
[the previous test](#indirect-command-processing-throughput)
except that it uses position attribute in vertex shader:

```c++
layout(location=0) in vec4 inPosition;
void main() {
	gl_Position = inPosition;
}
```

Vec4 coordinates are provided in a way to not produce any fragments in the rasterizer.
They are tiny triangles in between pixel sampling locations distributed roughly across the whole screen.


## Attribute and buffer performance

All these tests measure triangle rendering performance while using one to four attributes or buffers
in vertex shader.


### Attribute tests

For attribute tests, vertex shader looks like:

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
Its values specify tiny triangles in between pixel sampling locations distributed roughly
across the whole screen. So, they do not produce any fragments in the rasterizer.

This is used in the following tests:
- One attribute performance - 1x vec4 attribute
- Two attributes performance - 2x vec4 attribute
- Four attributes performance - 4x vec4 attribute


### Buffer tests

For buffer tests, vertex shader looks like:

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
Its values specify tiny triangles in between pixel sampling locations distributed roughly
across the whole screen. So, they do not produce any fragments in the rasterizer.

This is used in the following tests:
- One buffer performance - 1x vec4 buffer
- One buffer performance - 1x vec3 buffer
- Two buffers performance - 2x vec4 buffer
- Two buffers performance - 2x vec3 buffer
- Four buffers performance - 4x vec4 buffer
- Four buffers performance - 4x vec3 buffer


### Interleaved attribute tests

Interleaved attribute tests are using vertex input state that points all attributes to the same buffer.
All attributes for particular vertex are stored on the consecutive places in the same buffer:

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
that read various amount of interleaved data from the same buffer.
All the data for particular vertex are stored on the consecutive places in the same buffer.

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
distributed roughly across the whole screen.
They do not produce any fragments in the rasterizer.

This is used in the following tests:
- Two interleaved buffers performance - 2x vec4
- Four interleaved buffers performance - 4x vec4


### Packed data tests

These tests use one or two buffers which are unpacked into four attributes.

The first buffer typically contains x, y, and z coordinates as 4-byte floats.
The last four bytes are occupied by w coordinate and normal's z component,
both stored as half floats.

The second buffer typically contains texture u and v coordinates stored
as 4-byte floats, normal's x and y component stored as half floats,
and color stored as uint.

The unpacked data produce tiny triangles in between pixel sampling locations
distributed roughly across the whole screen.
They do not produce any fragments in the rasterizer.

#### Packed attribute performance - 2x uvec4 attribute unpacked

Packed attribute test uses two uvec4 attributes that are unpacked to four attributes:

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

More details can be found in [Packed data tests](#packed-data-tests) section.

#### Packed buffer performance - 1x buffer using 32-byte struct unpacked

Single packed buffer uses 32-byte struct stored in one buffer
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

More details can be found in [Packed data tests](#packed-data-tests) section.

#### Packed buffer performance - 2x uvec4 buffers unpacked

Packed buffer test uses two uvec4 buffers that are unpacked into four attributes:

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

More details can be found in [Packed data tests](#packed-data-tests) section.

#### Packed buffer performance - 2x buffer using 16-byte struct unpacked

Packed buffer test uses two structs stored in two buffers that are unpacked into four attributes:

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

More details can be found in [Packed data tests](#packed-data-tests) section.

#### Packed buffer performance - 2x buffer using 16-byte struct read multiple times and unpacked

Packed buffer test uses two structs stored in two buffers that are read multiple times
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

More details can be found in [Packed data tests](#packed-data-tests) section.

### Attribute conversion test

Used in the test:
- Four attributes performance - 2x vec4 and 2x R8G8B8A8 attribute

The code of the test is the same as [Attribute tests](#attribute-tests),
particularly Four attributes performance - 4x vec4 attribute,
with the exception that two buffers are not using VK_FORMAT_R32G32B32A32_SFLOAT
but VK_FORMAT_R8G8B8A8_UNORM in VkPipelineVertexInputStateCreateInfo:

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
