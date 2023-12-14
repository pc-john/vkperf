#include <array>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <random>
#include <regex>
#include <sstream>
#ifdef _WIN32
# include <windows.h>
# include <intrin.h>
#endif
#include <vulkan/vulkan.hpp>

using namespace std;


// constants
static const string appName = "vkperf";
static const uint32_t appVersion = VK_MAKE_VERSION(0,99,2);
static constexpr const vk::Extent2D defaultFramebufferExtent(1920,1080);  // FullHD resultion (allowed values are up to 4096x4096 which are guaranteed by Vulkan; for bigger values, test maxFramebufferWidth and maxFramebufferHeight of vk::PhysicalDeviceLimits)
static constexpr const double longTestTime = 60.;
static constexpr const double standardTestTime = 2.;
static constexpr const uint32_t numTrianglesStandard = uint32_t(1000*1e3);
static constexpr const uint32_t numTrianglesIntegratedGpu = uint32_t(100*1e3);
static constexpr const uint32_t numTrianglesCpu = uint32_t(10*1e3);
static constexpr const uint32_t numTrianglesMinimal = 2000;  // two times maxTriStripLength
static constexpr const uint32_t indirectRecordStride = 32;
static constexpr const unsigned triangleSize = 0;
static constexpr const uint32_t maxTriStripLength = 1000;  // length of triangle strip used during various measurements; some tests are splitting it to various lenght strips, while reusing two previous vertices from the previous strip


// Vulkan instance
// (must be destructed as the last one, at least on Linux, it must be destroyed after display connection)
static vk::UniqueInstance instance;

// Vulkan handles and objects
// (they need to be placed in particular (not arbitrary) order as it gives their destruction order)
static vk::PhysicalDevice physicalDevice;
static vk::PhysicalDeviceProperties physicalDeviceProperties;
static vector<vk::ExtensionProperties> physicalDeviceExtensions;
static uint32_t graphicsQueueFamily;
static uint32_t sparseQueueFamily;
static vk::PhysicalDeviceFeatures enabledFeatures;
static vk::UniqueDevice device;
static vk::Queue graphicsQueue;
static vk::Queue sparseQueue;
static constexpr const vk::Format colorFormat = vk::Format::eR8G8B8A8Srgb;
static vk::Format depthFormat;
static vk::UniqueRenderPass renderPass;
static vk::UniqueShaderModule attributelessConstantOutputVS;
static vk::UniqueShaderModule attributelessInputIndicesVS;
static vk::UniqueShaderModule coordinateAttributeVS;
static vk::UniqueShaderModule coordinate4BufferVS;
static vk::UniqueShaderModule coordinate3BufferVS;
static vk::UniqueShaderModule singleUniformMatrixVS;
static vk::UniqueShaderModule matrixAttributeVS;
static vk::UniqueShaderModule matrixBufferVS;
static vk::UniqueShaderModule twoAttributesVS;
static vk::UniqueShaderModule twoBuffersVS;
static vk::UniqueShaderModule twoBuffer3VS;
static vk::UniqueShaderModule twoInterleavedBuffersVS;
static vk::UniqueShaderModule twoPackedAttributesVS;
static vk::UniqueShaderModule twoPackedBuffersVS;
static vk::UniqueShaderModule twoPackedBuffersUsingStructVS;
static vk::UniqueShaderModule twoPackedBuffersUsingStructSlowVS;
static vk::UniqueShaderModule singlePackedBufferVS;
static vk::UniqueShaderModule twoPackedAttributesAndSingleMatrixVS;
static vk::UniqueShaderModule twoPackedAttributesAndMatrixVS;
static vk::UniqueShaderModule twoPackedBuffersAndMatrixVS;
static vk::UniqueShaderModule fourAttributesVS;
static vk::UniqueShaderModule fourBuffersVS;
static vk::UniqueShaderModule fourBuffer3VS;
static vk::UniqueShaderModule fourInterleavedBuffersVS;
static vk::UniqueShaderModule fourAttributesAndMatrixVS;
static vk::UniqueShaderModule geometryShaderConstantOutputVS;
static vk::UniqueShaderModule geometryShaderConstantOutputGS;
static vk::UniqueShaderModule geometryShaderConstantOutputTwoTrianglesGS;
static vk::UniqueShaderModule geometryShaderNoOutputGS;
static vk::UniqueShaderModule geometryShaderVS;
static vk::UniqueShaderModule geometryShaderGS;
static vk::UniqueShaderModule transformationThreeMatricesVS;
static vk::UniqueShaderModule transformationFiveMatricesVS;
static vk::UniqueShaderModule transformationFiveMatricesPushConstantsVS;
static vk::UniqueShaderModule transformationFiveMatricesSpecializationConstantsVS;
static vk::UniqueShaderModule transformationFiveMatricesConstantsVS;
static vk::UniqueShaderModule transformationFiveMatricesUsingGS;
static vk::UniqueShaderModule transformationFiveMatricesUsingGSAndAttributesVS;
static vk::UniqueShaderModule transformationFiveMatricesUsingGSAndAttributesGS;
static vk::UniqueShaderModule phongTexturedFourAttributesFiveMatricesVS;
static vk::UniqueShaderModule phongTexturedFourAttributesVS;
static vk::UniqueShaderModule phongTexturedVS;
static vk::UniqueShaderModule phongTexturedRowMajorVS;
static vk::UniqueShaderModule phongTexturedMat4x3VS;
static vk::UniqueShaderModule phongTexturedMat4x3RowMajorVS;
static vk::UniqueShaderModule phongTexturedQuat1VS;
static vk::UniqueShaderModule phongTexturedQuat2VS;
static vk::UniqueShaderModule phongTexturedQuat3VS;
static vk::UniqueShaderModule phongTexturedQuat2PrimitiveRestartVS;
static vk::UniqueShaderModule phongTexturedSingleQuat2VS;
static vk::UniqueShaderModule phongTexturedDMatricesOnlyInputVS;
static vk::UniqueShaderModule phongTexturedDMatricesVS;
static vk::UniqueShaderModule phongTexturedDMatricesDVerticesVS;
static vk::UniqueShaderModule phongTexturedInGSDMatricesDVerticesVS;
static vk::UniqueShaderModule phongTexturedInGSDMatricesDVerticesGS;
static vk::UniqueShaderModule constantColorFS;
static vk::UniqueShaderModule phongTexturedDummyFS;
static vk::UniqueShaderModule phongTexturedFS;
static vk::UniqueShaderModule phongTexturedNotPackedFS;
static vk::UniqueShaderModule fullscreenQuadVS;
static vk::UniqueShaderModule fullscreenQuadFourInterpolatorsVS;
static vk::UniqueShaderModule fullscreenQuadFourSmoothInterpolatorsFS;
static vk::UniqueShaderModule fullscreenQuadFourFlatInterpolatorsFS;
static vk::UniqueShaderModule fullscreenQuadTexturedPhongInterpolatorsVS;
static vk::UniqueShaderModule fullscreenQuadTexturedPhongInterpolatorsFS;
static vk::UniqueShaderModule uniformColor4fFS;
static vk::UniqueShaderModule uniformColor4bFS;
static vk::UniqueShaderModule fullscreenQuadTwoVec3InterpolatorsVS;
static vk::UniqueShaderModule phongNoSpecularFS;
static vk::UniqueShaderModule phongNoSpecularSingleUniformFS;
static vk::UniquePipelineCache pipelineCache;
static vk::UniquePipelineLayout simplePipelineLayout;
static vk::UniquePipelineLayout oneUniformVSPipelineLayout;
static vk::UniquePipelineLayout oneUniformFSPipelineLayout;
static vk::UniquePipelineLayout oneBufferPipelineLayout;
static vk::UniquePipelineLayout twoBuffersPipelineLayout;
static vk::UniquePipelineLayout threeBuffersPipelineLayout;
static vk::UniquePipelineLayout fourBuffersPipelineLayout;
static vk::UniquePipelineLayout threeBuffersInGSPipelineLayout;
static vk::UniquePipelineLayout threeUniformFSPipelineLayout;
static vk::UniquePipelineLayout bufferAndUniformPipelineLayout;
static vk::UniquePipelineLayout bufferAndUniformInGSPipelineLayout;
static vk::UniquePipelineLayout twoBuffersAndUniformPipelineLayout;
static vk::UniquePipelineLayout twoBuffersAndPushConstantsPipelineLayout;
static vk::UniquePipelineLayout twoBuffersAndUniformInGSPipelineLayout;
static vk::UniquePipelineLayout fourBuffersAndUniformInGSPipelineLayout;
static vk::UniquePipelineLayout phongTexturedPipelineLayout;
static vk::UniqueDescriptorSetLayout oneUniformVSDescriptorSetLayout;
static vk::UniqueDescriptorSetLayout oneUniformFSDescriptorSetLayout;
static vk::UniqueDescriptorSetLayout oneBufferDescriptorSetLayout;
static vk::UniqueDescriptorSetLayout twoBuffersDescriptorSetLayout;
static vk::UniqueDescriptorSetLayout threeBuffersDescriptorSetLayout;
static vk::UniqueDescriptorSetLayout fourBuffersDescriptorSetLayout;
static vk::UniqueDescriptorSetLayout threeBuffersInGSDescriptorSetLayout;
static vk::UniqueDescriptorSetLayout threeUniformFSDescriptorSetLayout;
static vk::UniqueDescriptorSetLayout bufferAndUniformDescriptorSetLayout;
static vk::UniqueDescriptorSetLayout bufferAndUniformInGSDescriptorSetLayout;
static vk::UniqueDescriptorSetLayout twoBuffersAndUniformDescriptorSetLayout;
static vk::UniqueDescriptorSetLayout twoBuffersAndUniformInGSDescriptorSetLayout;
static vk::UniqueDescriptorSetLayout fourBuffersAndUniformInGSDescriptorSetLayout;
static vk::UniqueDescriptorSetLayout phongTexturedDescriptorSetLayout;
static vk::UniqueBuffer singleMatrixUniformBuffer;
static vk::UniqueBuffer singlePATBuffer;
static vk::UniqueBuffer sameMatrixAttribute;  // not used now
static vk::UniqueBuffer sameMatrixBuffer;
static vk::UniqueBuffer sameMatrixRowMajorBuffer;
static vk::UniqueBuffer sameMatrix4x3Buffer;
static vk::UniqueBuffer sameMatrix4x3RowMajorBuffer;
static vk::UniqueBuffer sameDMatrixBuffer;
static vk::UniqueBuffer sameDMatrixStagingBuffer;
static vk::UniqueBuffer samePATBuffer;
static vk::UniqueBuffer transformationMatrixAttribute;
static vk::UniqueBuffer transformationMatrixBuffer;
static vk::UniqueBuffer indirectBuffer;
static vk::UniqueBuffer indirectStrideBuffer;
static vk::UniqueBuffer indirectIndexedBuffer;
static vk::UniqueBuffer indirectIndexedStrideBuffer;
static vk::UniqueBuffer normalMatrix4x3Buffer;
static vk::UniqueBuffer viewAndProjectionMatricesUniformBuffer;
static vk::UniqueBuffer viewAndProjectionDMatricesUniformBuffer;
static vk::UniqueBuffer materialUniformBuffer;
static vk::UniqueBuffer materialNotPackedUniformBuffer;
static vk::UniqueBuffer globalLightUniformBuffer;
static vk::UniqueBuffer lightUniformBuffer;
static vk::UniqueBuffer lightNotPackedUniformBuffer;
static vk::UniqueBuffer allInOneLightingUniformBuffer;
static vk::UniqueDeviceMemory singleMatrixUniformMemory;
static vk::UniqueDeviceMemory singlePATMemory;
static vk::UniqueDeviceMemory sameMatrixAttributeMemory;  // not used now
static vk::UniqueDeviceMemory sameMatrixBufferMemory;
static vk::UniqueDeviceMemory sameMatrixRowMajorBufferMemory;
static vk::UniqueDeviceMemory sameMatrix4x3BufferMemory;
static vk::UniqueDeviceMemory sameMatrix4x3RowMajorBufferMemory;
static vk::UniqueDeviceMemory sameDMatrixBufferMemory;
static vk::UniqueDeviceMemory sameDMatrixStagingBufferMemory;
static vk::UniqueDeviceMemory samePATBufferMemory;
static vk::UniqueDeviceMemory transformationMatrixAttributeMemory;
static vk::UniqueDeviceMemory transformationMatrixBufferMemory;  // not used now
static vk::UniqueDeviceMemory indirectBufferMemory;
static vk::UniqueDeviceMemory indirectStrideBufferMemory;
static vk::UniqueDeviceMemory indirectIndexedBufferMemory;
static vk::UniqueDeviceMemory indirectIndexedStrideBufferMemory;
static vk::UniqueDeviceMemory normalMatrix4x3Memory;
static vk::UniqueDeviceMemory viewAndProjectionMatricesMemory;
static vk::UniqueDeviceMemory viewAndProjectionDMatricesMemory;
static vk::UniqueDeviceMemory materialUniformBufferMemory;
static vk::UniqueDeviceMemory materialNotPackedUniformBufferMemory;
static vk::UniqueDeviceMemory globalLightUniformBufferMemory;
static vk::UniqueDeviceMemory lightUniformBufferMemory;
static vk::UniqueDeviceMemory lightNotPackedUniformBufferMemory;
static vk::UniqueDeviceMemory allInOneLightingUniformBufferMemory;
static vk::UniqueDescriptorPool descriptorPool;
static vk::DescriptorSet oneUniformVSDescriptorSet;
static vk::DescriptorSet one4fUniformFSDescriptorSet;
static vk::DescriptorSet one4bUniformFSDescriptorSet;
static vk::DescriptorSet coordinate4BufferDescriptorSet;
static vk::DescriptorSet coordinate3BufferDescriptorSet;
static vk::DescriptorSet sameMatrixBufferDescriptorSet;
static vk::DescriptorSet transformationMatrixBufferDescriptorSet;
static vk::DescriptorSet singlePackedBufferDescriptorSet;
static vk::DescriptorSet twoBuffersDescriptorSet;
static vk::DescriptorSet twoBuffer3DescriptorSet;
static vk::DescriptorSet twoInterleavedBuffersDescriptorSet;
static vk::DescriptorSet twoPackedBuffersDescriptorSet;
static vk::DescriptorSet threeBuffersDescriptorSet;
static vk::DescriptorSet fourBuffersDescriptorSet;
static vk::DescriptorSet fourBuffer3DescriptorSet;
static vk::DescriptorSet fourInterleavedBuffersDescriptorSet;
static vk::DescriptorSet threeBuffersInGSDescriptorSet;
static vk::DescriptorSet threeUniformFSDescriptorSet;
static vk::DescriptorSet transformationThreeMatricesDescriptorSet;
static vk::DescriptorSet transformationThreeMatricesRowMajorDescriptorSet;
static vk::DescriptorSet transformationThreeMatrices4x3DescriptorSet;
static vk::DescriptorSet transformationThreeMatrices4x3RowMajorDescriptorSet;
static vk::DescriptorSet transformationThreeDMatricesDescriptorSet;
static vk::DescriptorSet transformationTwoMatricesDescriptorSet;
static vk::DescriptorSet transformationTwoMatricesAndPATDescriptorSet;
static vk::DescriptorSet transformationTwoMatricesAndSinglePATDescriptorSet;
static vk::DescriptorSet transformationFiveMatricesDescriptorSet;
static vk::DescriptorSet transformationFiveMatricesUsingGSDescriptorSet;
static vk::DescriptorSet transformationFiveMatricesUsingGSAndAttributesDescriptorSet;
static vk::DescriptorSet phongTexturedThreeDMatricesUsingGSAndAttributesDescriptorSet;
static vk::DescriptorSet phongTexturedDescriptorSet;
static vk::DescriptorSet phongTexturedNotPackedDescriptorSet;
static vk::DescriptorSet allInOneLightingUniformDescriptorSet;
static vk::UniqueFramebuffer framebuffer;
static vk::UniqueImage colorImage;
static vk::UniqueImage depthImage;
static vk::UniqueDeviceMemory colorImageMemory;
static vk::UniqueDeviceMemory depthImageMemory;
static vk::UniqueImageView colorImageView;
static vk::UniqueImageView depthImageView;
static vk::UniqueFence fence;
static vk::UniquePipeline attributelessConstantOutputPipeline;
static vk::UniquePipeline attributelessConstantOutputTriStripPipeline;
static vk::UniquePipeline attributelessConstantOutputPrimitiveRestartPipeline;
static vk::UniquePipeline attributelessInputIndicesPipeline;
static vk::UniquePipeline attributelessInputIndicesTriStripPipeline;
static vk::UniquePipeline attributelessInputIndicesPrimitiveRestartPipeline;
static vk::UniquePipeline coordinateAttributePipeline;
static vk::UniquePipeline coordinate4BufferPipeline;
static vk::UniquePipeline coordinate3BufferPipeline;
static vk::UniquePipeline singleMatrixUniformPipeline;
static vk::UniquePipeline matrixAttributePipeline;
static vk::UniquePipeline matrixBufferPipeline;
static vk::UniquePipeline twoAttributesPipeline;
static vk::UniquePipeline twoBuffersPipeline;
static vk::UniquePipeline twoBuffer3Pipeline;
static vk::UniquePipeline twoInterleavedAttributesPipeline;
static vk::UniquePipeline twoInterleavedBuffersPipeline;
static vk::UniquePipeline twoPackedAttributesPipeline;
static vk::UniquePipeline twoPackedBuffersPipeline;
static vk::UniquePipeline twoPackedBuffersUsingStructPipeline;
static vk::UniquePipeline twoPackedBuffersUsingStructSlowPipeline;
static vk::UniquePipeline two4F32Two4U8AttributesPipeline;
static vk::UniquePipeline singlePackedBufferPipeline;
static vk::UniquePipeline twoPackedAttributesAndSingleMatrixPipeline;
static vk::UniquePipeline twoPackedAttributesAndMatrixPipeline;
static vk::UniquePipeline twoPackedBuffersAndMatrixPipeline;
static vk::UniquePipeline fourAttributesPipeline;
static vk::UniquePipeline fourBuffersPipeline;
static vk::UniquePipeline fourBuffer3Pipeline;
static vk::UniquePipeline fourInterleavedAttributesPipeline;
static vk::UniquePipeline fourInterleavedBuffersPipeline;
static vk::UniquePipeline fourAttributesAndMatrixPipeline;
static vk::UniquePipeline geometryShaderConstantOutputPipeline;
static vk::UniquePipeline geometryShaderConstantOutputTwoTrianglesPipeline;
static vk::UniquePipeline geometryShaderNoOutputPipeline;
static vk::UniquePipeline geometryShaderPipeline;
static vk::UniquePipeline transformationThreeMatricesPipeline;
static vk::UniquePipeline transformationFiveMatricesPipeline;
static vk::UniquePipeline transformationFiveMatricesPushConstantsPipeline;
static vk::UniquePipeline transformationFiveMatricesSpecializationConstantsPipeline;
static vk::UniquePipeline transformationFiveMatricesConstantsPipeline;
static vk::UniquePipeline transformationFiveMatricesUsingGSPipeline;
static vk::UniquePipeline transformationFiveMatricesUsingGSAndAttributesPipeline;
static vk::UniquePipeline phongTexturedFourAttributesFiveMatricesPipeline;
static vk::UniquePipeline phongTexturedFourAttributesPipeline;
static vk::UniquePipeline phongTexturedPipeline;
static vk::UniquePipeline phongTexturedRowMajorPipeline;
static vk::UniquePipeline phongTexturedMat4x3Pipeline;
static vk::UniquePipeline phongTexturedMat4x3RowMajorPipeline;
static vk::UniquePipeline phongTexturedQuat1Pipeline;
static vk::UniquePipeline phongTexturedQuat2Pipeline;
static vk::UniquePipeline phongTexturedQuat3Pipeline;
static vk::UniquePipeline phongTexturedQuat2PrimitiveRestartPipeline;
static vk::UniquePipeline phongTexturedDMatricesOnlyInputPipeline;
static vk::UniquePipeline phongTexturedDMatricesPipeline;
static vk::UniquePipeline phongTexturedDMatricesDVerticesPipeline;
static vk::UniquePipeline phongTexturedInGSDMatricesDVerticesPipeline;
static vk::UniquePipeline phongTexturedSingleQuat2Pipeline;
static vk::UniquePipeline phongTexturedSingleQuat2TriStripPipeline;
static vk::UniquePipeline phongTexturedSingleQuat2PrimitiveRestartPipeline;
static vk::UniquePipeline fillrateContantColorPipeline;
static vk::UniquePipeline fillrateFourSmoothInterpolatorsPipeline;
static vk::UniquePipeline fillrateFourFlatInterpolatorsPipeline;
static vk::UniquePipeline fillrateTexturedPhongInterpolatorsPipeline;
static vk::UniquePipeline fillrateTexturedPhongPipeline;
static vk::UniquePipeline fillrateTexturedPhongNotPackedPipeline;
static vk::UniquePipeline fillrateUniformColor4fPipeline;
static vk::UniquePipeline fillrateUniformColor4bPipeline;
static vk::UniquePipeline phongNoSpecularPipeline;
static vk::UniquePipeline phongNoSpecularSingleUniformPipeline;
static vk::UniqueCommandPool commandPool;
static vk::UniqueCommandBuffer commandBuffer;
static vk::UniqueBuffer coordinate4Attribute;
static vk::UniqueBuffer coordinate4Buffer;
static vk::UniqueBuffer coordinate3Attribute;
static vk::UniqueBuffer coordinate3Buffer;
static vk::UniqueBuffer normalAttribute;
static vk::UniqueBuffer colorAttribute;
static vk::UniqueBuffer texCoordAttribute;
static array<vk::UniqueBuffer,3> vec4Attributes;
static array<vk::UniqueBuffer,3> vec4Buffers;
static array<vk::UniqueBuffer,2> vec4u8Attributes;
static array<vk::UniqueBuffer,3> vec3Buffers;
static vk::UniqueBuffer packedAttribute1;
static vk::UniqueBuffer packedAttribute2;
static vk::UniqueBuffer twoInterleavedAttributes;
static vk::UniqueBuffer twoInterleavedBuffers;
static vk::UniqueBuffer fourInterleavedAttributes;
static vk::UniqueBuffer fourInterleavedBuffers;
static vk::UniqueBuffer packedBuffer1;
static vk::UniqueBuffer packedBuffer2;
static vk::UniqueBuffer singlePackedBuffer;
static vk::UniqueBuffer packedDAttribute1;
static vk::UniqueBuffer packedDAttribute2;
static vk::UniqueBuffer packedDAttribute3;
static vk::UniqueBuffer indexBuffer;
static vk::UniqueBuffer primitiveRestartIndexBuffer;
static vk::UniqueBuffer stripIndexBuffer;
static vk::UniqueBuffer stripPrimitiveRestart3IndexBuffer;
static vk::UniqueBuffer stripPrimitiveRestart4IndexBuffer;
static vk::UniqueBuffer stripPrimitiveRestart7IndexBuffer;
static vk::UniqueBuffer stripPrimitiveRestart10IndexBuffer;
static vk::UniqueBuffer stripPrimitiveRestart1002IndexBuffer;
static vk::UniqueBuffer primitiveRestartMinusOne2IndexBuffer;
static vk::UniqueBuffer primitiveRestartMinusOne5IndexBuffer;
static vk::UniqueBuffer minusOneIndexBuffer;
static vk::UniqueBuffer zeroIndexBuffer;
static vk::UniqueBuffer plusOneIndexBuffer;
static vk::UniqueBuffer stripPackedAttribute1;
static vk::UniqueBuffer stripPackedAttribute2;
static vk::UniqueBuffer sharedVertexPackedAttribute1;
static vk::UniqueBuffer sharedVertexPackedAttribute2;
static vk::UniqueBuffer sameVertexPackedAttribute1;
static vk::UniqueBuffer sameVertexPackedAttribute2;
static vk::UniqueDeviceMemory coordinate4AttributeMemory;
static vk::UniqueDeviceMemory coordinate4BufferMemory;
static vk::UniqueDeviceMemory coordinate3AttributeMemory;
static vk::UniqueDeviceMemory coordinate3BufferMemory;
static vk::UniqueDeviceMemory normalAttributeMemory;
static vk::UniqueDeviceMemory colorAttributeMemory;
static vk::UniqueDeviceMemory texCoordAttributeMemory;
static array<vk::UniqueDeviceMemory,3> vec4AttributeMemory;
static array<vk::UniqueDeviceMemory,2> vec4u8AttributeMemory;
static array<vk::UniqueDeviceMemory,3> vec4BufferMemory;
static array<vk::UniqueDeviceMemory,3> vec3BufferMemory;
static vk::UniqueDeviceMemory packedAttribute1Memory;
static vk::UniqueDeviceMemory packedAttribute2Memory;
static vk::UniqueDeviceMemory twoInterleavedAttributesMemory;
static vk::UniqueDeviceMemory twoInterleavedBuffersMemory;
static vk::UniqueDeviceMemory fourInterleavedAttributesMemory;
static vk::UniqueDeviceMemory fourInterleavedBuffersMemory;
static vk::UniqueDeviceMemory packedBuffer1Memory;
static vk::UniqueDeviceMemory packedBuffer2Memory;
static vk::UniqueDeviceMemory singlePackedBufferMemory;
static vk::UniqueDeviceMemory packedDAttribute1Memory;
static vk::UniqueDeviceMemory packedDAttribute2Memory;
static vk::UniqueDeviceMemory packedDAttribute3Memory;
static vk::UniqueDeviceMemory indexBufferMemory;
static vk::UniqueDeviceMemory primitiveRestartIndexBufferMemory;
static vk::UniqueDeviceMemory stripIndexBufferMemory;
static vk::UniqueDeviceMemory stripPrimitiveRestart3IndexBufferMemory;
static vk::UniqueDeviceMemory stripPrimitiveRestart4IndexBufferMemory;
static vk::UniqueDeviceMemory stripPrimitiveRestart7IndexBufferMemory;
static vk::UniqueDeviceMemory stripPrimitiveRestart10IndexBufferMemory;
static vk::UniqueDeviceMemory stripPrimitiveRestart1002IndexBufferMemory;
static vk::UniqueDeviceMemory primitiveRestartMinusOne2IndexBufferMemory;
static vk::UniqueDeviceMemory primitiveRestartMinusOne5IndexBufferMemory;
static vk::UniqueDeviceMemory minusOneIndexBufferMemory;
static vk::UniqueDeviceMemory zeroIndexBufferMemory;
static vk::UniqueDeviceMemory plusOneIndexBufferMemory;
static vk::UniqueDeviceMemory stripPackedAttribute1Memory;
static vk::UniqueDeviceMemory stripPackedAttribute2Memory;
static vk::UniqueDeviceMemory sharedVertexPackedAttribute1Memory;
static vk::UniqueDeviceMemory sharedVertexPackedAttribute2Memory;
static vk::UniqueDeviceMemory sameVertexPackedAttribute1Memory;
static vk::UniqueDeviceMemory sameVertexPackedAttribute2Memory;
static vk::UniqueImage singleTexelImage;
static vk::UniqueDeviceMemory singleTexelImageMemory;
static vk::UniqueImageView singleTexelImageView;
static vk::UniqueSampler trilinearSampler;
static vk::UniqueQueryPool timestampPool;
static uint32_t timestampValidBits=0;
static float timestampPeriod_ns=0;
static uint32_t numTriangles;
static bool minimalTest=false;
static bool longTest=false;
static bool runAllTests=false;
static bool debug=false;
static vk::Extent2D framebufferExtent(0,0);
static size_t sameDMatrixStagingBufferSize;
static uint32_t numFullscreenQuads=10; // note: if you increase the value, make sure that fullscreenQuad*.vert is still drawing to the clip space (by gl_InstanceIndex)

// sparse memory variables
enum { SPARSE_NONE, SPARSE_BINDING, SPARSE_RESIDENCY, SPARSE_RESIDENCY_ALIASED };
static int sparseMode = SPARSE_NONE;
static size_t memoryBlockSize=~0;
static size_t memoryBlockMask=~0;
static size_t sparseBlockSize=~0;
static vk::BufferCreateFlags bufferCreateFlags = {};
static unsigned bufferSizeMultiplier = 1;

// shader code in SPIR-V binary
static const uint32_t attributelessConstantOutputVS_spirv[]={
#include "attributelessConstantOutput.vert.spv"
};
static const uint32_t attributelessInputIndicesVS_spirv[]={
#include "attributelessInputIndices.vert.spv"
};
static const uint32_t coordinateAttributeVS_spirv[]={
#include "coordinateAttribute.vert.spv"
};
static const uint32_t coordinate4BufferVS_spirv[]={
#include "coordinate4Buffer.vert.spv"
};
static const uint32_t coordinate3BufferVS_spirv[]={
#include "coordinate3Buffer.vert.spv"
};
static const uint32_t singleUniformMatrixVS_spirv[]={
#include "singleUniformMatrix.vert.spv"
};
static const uint32_t matrixAttributeVS_spirv[]={
#include "matrixAttribute.vert.spv"
};
static const uint32_t matrixBufferVS_spirv[]={
#include "matrixBuffer.vert.spv"
};
static const uint32_t twoAttributesVS_spirv[]={
#include "twoAttributes.vert.spv"
};
static const uint32_t twoBuffersVS_spirv[]={
#include "twoBuffers.vert.spv"
};
static const uint32_t twoBuffer3VS_spirv[]={
#include "twoBuffer3.vert.spv"
};
static const uint32_t twoInterleavedBuffersVS_spirv[]={
#include "twoInterleavedBuffers.vert.spv"
};
static const uint32_t twoPackedAttributesVS_spirv[]={
#include "twoPackedAttributes.vert.spv"
};
static const uint32_t twoPackedBuffersVS_spirv[]={
#include "twoPackedBuffers.vert.spv"
};
static const uint32_t twoPackedBuffersUsingStructVS_spirv[]={
#include "twoPackedBuffersUsingStruct.vert.spv"
};
static const uint32_t twoPackedBuffersUsingStructSlowVS_spirv[]={
#include "twoPackedBuffersUsingStructSlow.vert.spv"
};
static const uint32_t singlePackedBufferVS_spirv[]={
#include "singlePackedBuffer.vert.spv"
};
static const uint32_t twoPackedAttributesAndSingleMatrixVS_spirv[]={
#include "twoPackedAttributesAndSingleMatrix.vert.spv"
};
static const uint32_t twoPackedAttributesAndMatrixVS_spirv[]={
#include "twoPackedAttributesAndMatrix.vert.spv"
};
static const uint32_t twoPackedBuffersAndMatrixVS_spirv[]={
#include "twoPackedBuffersAndMatrix.vert.spv"
};
static const uint32_t fourAttributesVS_spirv[]={
#include "fourAttributes.vert.spv"
};
static const uint32_t fourBuffersVS_spirv[]={
#include "fourBuffers.vert.spv"
};
static const uint32_t fourBuffer3VS_spirv[]={
#include "fourBuffer3.vert.spv"
};
static const uint32_t fourInterleavedBuffersVS_spirv[]={
#include "fourInterleavedBuffers.vert.spv"
};
static const uint32_t fourAttributesAndMatrixVS_spirv[]={
#include "fourAttributesAndMatrix.vert.spv"
};
static const uint32_t geometryShaderConstantOutputVS_spirv[]={
#include "geometryShaderConstantOutput.vert.spv"
};
static const uint32_t geometryShaderNoOutputGS_spirv[]={
#include "geometryShaderNoOutput.geom.spv"
};
static const uint32_t geometryShaderConstantOutputGS_spirv[]={
#include "geometryShaderConstantOutput.geom.spv"
};
static const uint32_t geometryShaderConstantOutputTwoTrianglesGS_spirv[]={
#include "geometryShaderConstantOutputTwoTriangles.geom.spv"
};
static const uint32_t geometryShaderVS_spirv[]={
#include "geometryShader.vert.spv"
};
static const uint32_t geometryShaderGS_spirv[]={
#include "geometryShader.geom.spv"
};
static const uint32_t transformationThreeMatricesVS_spirv[]={
#include "transformationThreeMatrices.vert.spv"
};
static const uint32_t transformationFiveMatricesVS_spirv[]={
#include "transformationFiveMatrices.vert.spv"
};
static const uint32_t transformationFiveMatricesPushConstantsVS_spirv[]={
#include "transformationFiveMatrices-pushConstants.vert.spv"
};
static const uint32_t transformationFiveMatricesSpecializationConstantsVS_spirv[]={
#include "transformationFiveMatrices-specializationConstants.vert.spv"
};
static const uint32_t transformationFiveMatricesConstantsVS_spirv[]={
#include "transformationFiveMatrices-constants.vert.spv"
};
static const uint32_t transformationFiveMatricesUsingGS_spirv[]={
#include "transformationFiveMatricesUsingGS.geom.spv"
};
static const uint32_t transformationFiveMatricesUsingGSAndAttributesVS_spirv[]={
#include "transformationFiveMatricesUsingGSAndAttributes.vert.spv"
};
static const uint32_t transformationFiveMatricesUsingGSAndAttributesGS_spirv[]={
#include "transformationFiveMatricesUsingGSAndAttributes.geom.spv"
};
static const uint32_t phongTexturedFourAttributesFiveMatricesVS_spirv[]={
#include "phongTexturedFourAttributesFiveMatrices.vert.spv"
};
static const uint32_t phongTexturedFourAttributesVS_spirv[]={
#include "phongTexturedFourAttributes.vert.spv"
};
static const uint32_t phongTexturedVS_spirv[]={
#include "phongTextured.vert.spv"
};
static const uint32_t phongTexturedRowMajorVS_spirv[]={
#include "phongTexturedRowMajor.vert.spv"
};
static const uint32_t phongTexturedMat4x3VS_spirv[]={
#include "phongTexturedMat4x3.vert.spv"
};
static const uint32_t phongTexturedMat4x3RowMajorVS_spirv[]={
#include "phongTexturedMat4x3RowMajor.vert.spv"
};
static const uint32_t phongTexturedQuat1VS_spirv[]={
#include "phongTexturedQuat1.vert.spv"
};
static const uint32_t phongTexturedQuat2VS_spirv[]={
#include "phongTexturedQuat2.vert.spv"
};
static const uint32_t phongTexturedQuat3VS_spirv[]={
#include "phongTexturedQuat3.vert.spv"
};
static const uint32_t phongTexturedQuat2PrimitiveRestartVS_spirv[]={
#include "phongTexturedQuat2PrimitiveRestart.vert.spv"
};
static const uint32_t phongTexturedSingleQuat2VS_spirv[]={
#include "phongTexturedSingleQuat2.vert.spv"
};
static const uint32_t phongTexturedDMatricesOnlyInputVS_spirv[]={
#include "phongTexturedDMatricesOnlyInput.vert.spv"
};
static const uint32_t phongTexturedDMatricesVS_spirv[]={
#include "phongTexturedDMatrices.vert.spv"
};
static const uint32_t phongTexturedDMatricesDVerticesVS_spirv[]={
#include "phongTexturedDMatricesDVertices.vert.spv"
};
static const uint32_t phongTexturedInGSDMatricesDVerticesVS_spirv[]={
#include "phongTexturedInGSDMatricesDVertices.vert.spv"
};
static const uint32_t phongTexturedInGSDMatricesDVerticesGS_spirv[]={
#include "phongTexturedInGSDMatricesDVertices.geom.spv"
};
static const uint32_t constantColorFS_spirv[]={
#include "constantColor.frag.spv"
};
static const uint32_t phongTexturedDummyFS_spirv[]={
#include "phongTexturedDummy.frag.spv"
};
static const uint32_t phongTexturedFS_spirv[]={
#include "phongTextured.frag.spv"
};
static const uint32_t phongTexturedNotPackedFS_spirv[]={
#include "phongTexturedNotPacked.frag.spv"
};
static const uint32_t fullscreenQuadVS_spirv[]={
#include "fullscreenQuad.vert.spv"
};
static const uint32_t fullscreenQuadFourInterpolatorsVS_spirv[]={
#include "fullscreenQuadFourInterpolators.vert.spv"
};
static const uint32_t fullscreenQuadFourSmoothInterpolatorsFS_spirv[]={
#include "fullscreenQuadFourSmoothInterpolators.frag.spv"
};
static const uint32_t fullscreenQuadFourFlatInterpolatorsFS_spirv[]={
#include "fullscreenQuadFourFlatInterpolators.frag.spv"
};
static const uint32_t fullscreenQuadTexturedPhongInterpolatorsVS_spirv[]={
#include "fullscreenQuadTexturedPhongInterpolators.vert.spv"
};
static const uint32_t fullscreenQuadTexturedPhongInterpolatorsFS_spirv[]={
#include "fullscreenQuadTexturedPhongInterpolators.frag.spv"
};
static const uint32_t uniformColor4fFS_spirv[]={
#include "uniformColor4f.frag.spv"
};
static const uint32_t uniformColor4bFS_spirv[]={
#include "uniformColor4b.frag.spv"
};
static const uint32_t fullscreenQuadTwoVec3InterpolatorsVS_spirv[]={
#include "fullscreenQuadTwoVec3Interpolators.vert.spv"
};
static const uint32_t phongNoSpecularFS_spirv[]={
#include "phongNoSpecular.frag.spv"
};
static const uint32_t phongNoSpecularSingleUniformFS_spirv[]={
#include "phongNoSpecularSingleUniform.frag.spv"
};


struct Test {
	vector<uint64_t> renderingTimes;
	uint32_t timestampIndex;
	const char* groupText = nullptr;
	uint32_t groupVariable;
	string text;
	bool enabled = true;
	enum class Type { WarmUp, VertexThroughput, FragmentThroughput, TransferThroughput };
	Type type;
	union {
		double numRenderedItems;
		size_t numTransfers;
	};
	size_t transferSize;
	typedef void (*Func)(vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t groupVariable);
	Func func;
	Test(const char* text_, Type t, Func func_) : text(text_), type(t), func(func_)  {}
	Test(const char* groupText_, uint32_t groupVariable_, const char* text_, Type t, Func func_) : groupText(groupText_), groupVariable(groupVariable_), text(text_), type(t), func(func_)  {}
};

static vector<Test> tests;
static vector<Test*> shuffledTests;


static void beginTestBarrier(vk::CommandBuffer cb)
{
	cb.pipelineBarrier(
		vk::PipelineStageFlagBits::eAllCommands,  // srcStageMask
		vk::PipelineStageFlagBits::eAllCommands,  // dstStageMask
		vk::DependencyFlags(),  // dependencyFlags
		1,
		array{  // memoryBarrierCount+pMemoryBarriers
			vk::MemoryBarrier(
				vk::AccessFlagBits::eMemoryRead|vk::AccessFlagBits::eMemoryWrite,  // srcAccessMask
				vk::AccessFlagBits::eMemoryRead|vk::AccessFlagBits::eMemoryWrite  // dstAccessMask
			),
		}.data(),
		0,nullptr,  // bufferMemoryBarrierCount+pBufferMemoryBarriers
		0,nullptr   // imageMemoryBarrierCount+pImageMemoryBarriers
	);
}


static void beginTest(
	vk::CommandBuffer cb, vk::Pipeline pipeline, vk::PipelineLayout pipelineLayout, uint32_t& timestampIndex,
	const vector<vk::Buffer>& attributes, const vector<vk::DescriptorSet>& descriptorSets)
{
	beginTestBarrier(cb);
	cb.beginRenderPass(
		vk::RenderPassBeginInfo(
			renderPass.get(),         // renderPass
			framebuffer.get(),        // framebuffer
			vk::Rect2D(vk::Offset2D(0,0),framebufferExtent),  // renderArea
			2,                        // clearValueCount
			array<vk::ClearValue,2>{  // pClearValues
				vk::ClearColorValue(array<float,4>{0.f,0.f,0.f,1.f}),
				vk::ClearDepthStencilValue(1.f,0)
			}.data()
		),
		vk::SubpassContents::eInline
	);
	cb.bindPipeline(vk::PipelineBindPoint::eGraphics,pipeline);  // bind pipeline
	if(descriptorSets.size()>0)
		cb.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics,  // pipelineBindPoint
			pipelineLayout,  // layout
			0,  // firstSet
			descriptorSets,  // descriptorSets
			nullptr  // dynamicOffsets
		);
	if(attributes.size()>0)
		cb.bindVertexBuffers(
			0,  // firstBinding
			uint32_t(attributes.size()),  // bindingCount
			attributes.data(),  // pBuffers
			vector<vk::DeviceSize>(attributes.size(),0).data()  // pOffsets
		);
	cb.writeTimestamp(
		vk::PipelineStageFlagBits::eTopOfPipe,  // pipelineStage
		timestampPool.get(),  // queryPool
		timestampIndex++      // query
	);
}


static void endTest(vk::CommandBuffer cb, uint32_t& timestampIndex)
{
	cb.writeTimestamp(
		vk::PipelineStageFlagBits::eColorAttachmentOutput,  // pipelineStage
		timestampPool.get(),  // queryPool
		timestampIndex++      // query
	);
	cb.endRenderPass();
}


static void initTests()
{
	tests.clear();
	tests.reserve(200);

	tests.emplace_back(
		"   Test just to warm up GPU. The test shall be invisible to the user.",
		Test::Type::WarmUp,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			// render something to put GPU out of power saving states
			beginTest(cb, coordinateAttributePipeline.get(), simplePipelineLayout.get(), timestampIndex,
					  vector<vk::Buffer>{ coordinate4Attribute.get() },
					  vector<vk::DescriptorSet>());
			cb.draw(3*numTriangles,1,0,0);
			cb.draw(3*numTriangles,1,0,0);
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   VS max throughput for triangle list (single per-scene vkCmdDraw() call,\n"
		"      attributeless, constant VS output):      ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, attributelessConstantOutputPipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   VS max throughput for indexed triangle list (single per-scene\n"
		"      vkCmdDrawIndexed() call, monotonically increasing indices,\n"
		"      attributeless, constant VS output):      ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			cb.bindIndexBuffer(indexBuffer.get(), 0, vk::IndexType::eUint32);
			beginTest(cb, attributelessConstantOutputPipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			cb.drawIndexed(3*numTriangles, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   VS max throughput for indexed triangle list that reuses two indices from\n"
		"      the previous triangle (single per-scene vkCmdDrawIndexed() call,\n"
		"      monotonically increasing indices,\n"
		"      attributeless, constant VS output):      ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			cb.bindIndexBuffer(stripIndexBuffer.get(), 0, vk::IndexType::eUint32);
			beginTest(cb, attributelessConstantOutputPipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			cb.drawIndexed(3*numTriangles, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
			endTest(cb, timestampIndex);
		}
	);

#if 0 // not needed because it was replaced by the test bellow that uses strips of various lengths
	tests.emplace_back(
		"   VS max throughput for triangle strip\n"
		"      (per-strip vkCmdDraw() call, 1000 triangles per strip,\n"
		"      attributeless, constant VS output):      ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, attributelessConstantOutputTriStripPipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			for(uint32_t i=0,e=(numTriangles/maxTriStripLength)*(2+maxTriStripLength); i<e; i+=2+maxTriStripLength)
				cb.draw(2+maxTriStripLength, 1, i, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);
#endif

	const char* triStripPerformanceText =
		"   VS max throughput for triangle strips of various lengths\n"
		"      (per-strip vkCmdDraw() call, 1 to 1000 triangles per strip,\n"
		"      attributeless, constant VS output):      ";
	for(uint32_t n : array<uint32_t,12>{1,2,5,8,10,20,25,40,50,100,125,maxTriStripLength}) {  // numbers in this list needs to be divisible by maxTriStripLength (1000 by default) with reminder zero

		tests.emplace_back(
			triStripPerformanceText,
			n,
			[](uint32_t n) {
				string s = static_cast<stringstream&&>(stringstream() << "         strip length " << n << ": ").str();
				s.append(28-s.size(), ' ');
				return s;
			}(n).c_str(),
			Test::Type::VertexThroughput,
			[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t n)
			{
				beginTest(cb, attributelessConstantOutputTriStripPipeline.get(), simplePipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>(),
				          vector<vk::DescriptorSet>());
				for(uint32_t i=0,e=(numTriangles/maxTriStripLength)*(2+maxTriStripLength); i<e; i+=2+maxTriStripLength)
					for(uint32_t j=i,je=i+maxTriStripLength; j<je; j+=n)
						cb.draw(n+2, 1, j, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
				endTest(cb, timestampIndex);
			}
		);
	}

#if 0 // not needed because it was replaced by the test bellow that uses strips of various lengths
	tests.emplace_back(
		"   VS max throughput for indexed triangle strip\n"
		"      (per-strip vkCmdDrawIndexed() call, 1000 triangles per strip,\n"
		"      monotonically increasing indices,\n"
		"      attributeless, constant VS output):      ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			cb.bindIndexBuffer(indexBuffer.get(), 0, vk::IndexType::eUint32);
			beginTest(cb, attributelessConstantOutputTriStripPipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			for(uint32_t i=0,e=(numTriangles/maxTriStripLength)*(2+maxTriStripLength); i<e; i+=2+maxTriStripLength)
				cb.drawIndexed(2+maxTriStripLength, 1, i, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
			endTest(cb, timestampIndex);
		}
	);
#endif

	const char* indexedTriStripPerformanceText =
		"   VS max throughput for indexed triangle strips of various lengths\n"
		"      (per-strip vkCmdDrawIndexed() call, 1-1000 triangles per strip,\n"
		"      monotonically increasing indices,\n"
		"      attributeless, constant VS output):      ";
	for(uint32_t n : array<uint32_t,12>{1,2,5,8,10,20,25,40,50,100,125,maxTriStripLength}) {  // numbers in this list needs to be divisible by maxTriStripLength (1000 by default) with reminder zero

		tests.emplace_back(
			indexedTriStripPerformanceText,
			n,
			[](uint32_t n) {
				string s = static_cast<stringstream&&>(stringstream() << "         strip length " << n << ": ").str();
				s.append(28-s.size(), ' ');
				return s;
			}(n).c_str(),
			Test::Type::VertexThroughput,
			[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t n)
			{
				cb.bindIndexBuffer(indexBuffer.get(), 0, vk::IndexType::eUint32);
				beginTest(cb, attributelessConstantOutputTriStripPipeline.get(), simplePipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>(),
				          vector<vk::DescriptorSet>());
				for(uint32_t i=0,e=(numTriangles/maxTriStripLength)*(2+maxTriStripLength); i<e; i+=2+maxTriStripLength)
					for(uint32_t j=i,je=i+maxTriStripLength; j<je; j+=n)
						cb.drawIndexed(n+2, 1, j, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
				endTest(cb, timestampIndex);
			}
		);
	}

#if 0 // not needed because it was replaced by the test bellow that uses strips of various lengths
	tests.emplace_back(
		"   VS max throughput for primitive restart indexed triangle strip\n"
		"      (single per-scene vkCmdDrawIndexed() call, 1000 triangles per strip\n"
		"      followed by -1, monotonically increasing indices,\n"
		"      attributeless, constant VS output):      ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			cb.bindIndexBuffer(stripPrimitiveRestart1002IndexBuffer.get(), 0, vk::IndexType::eUint32);
			beginTest(cb, attributelessConstantOutputPrimitiveRestartPipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			uint32_t numIndicesPerStrip = maxTriStripLength+3;
			uint32_t numStrips = numTriangles/maxTriStripLength;
			cb.drawIndexed(numIndicesPerStrip*numStrips, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
			endTest(cb, timestampIndex);
		}
	);
#endif

	const char* primitiveRestartPerformanceText =
		"   VS max throughput for primitive restart indexed triangle strips of various\n"
		"      lengths (single per-scene vkCmdDrawIndexed() call, 1-1000 triangles per\n"
		"      strip, each strip finished by -1, monotonically increasing indices,\n"
		"      attributeless, constant VS output):      ";
	for(uint32_t n : array<uint32_t,5>{1,2,5,8,1000}) {  // numbers in this list needs to be divisible by maxTriStripLength (1000 by default) with reminder zero

		tests.emplace_back(
			primitiveRestartPerformanceText,
			n,
			[](uint32_t n) {
				string s = static_cast<stringstream&&>(stringstream()
				           << "         strip length " << n << ": ").str();
				if(s.size()<28)
					s.append(28-s.size(), ' ');
				return s;
			}(n).c_str(),
			Test::Type::VertexThroughput,
			[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t triPerStrip)
			{
				switch(triPerStrip) {
				case 1: cb.bindIndexBuffer(stripPrimitiveRestart3IndexBuffer.get(), 0, vk::IndexType::eUint32); break;
				case 2: cb.bindIndexBuffer(stripPrimitiveRestart4IndexBuffer.get(), 0, vk::IndexType::eUint32); break;
				case 5: cb.bindIndexBuffer(stripPrimitiveRestart7IndexBuffer.get(), 0, vk::IndexType::eUint32); break;
				case 8: cb.bindIndexBuffer(stripPrimitiveRestart10IndexBuffer.get(), 0, vk::IndexType::eUint32); break;
				case 1000: cb.bindIndexBuffer(stripPrimitiveRestart1002IndexBuffer.get(), 0, vk::IndexType::eUint32); break;
				default: assert(0 && "Unhandled triPerStrip parameter."); return;
				};
				beginTest(cb, attributelessConstantOutputPrimitiveRestartPipeline.get(), simplePipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>(),
				          vector<vk::DescriptorSet>());
				uint32_t numIndicesPerStrip = triPerStrip+3;
				uint32_t numStrips = numTriangles/triPerStrip;
				cb.drawIndexed(numIndicesPerStrip*numStrips, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
				endTest(cb, timestampIndex);
			}
		);
	}

	tests.emplace_back(
		"   VS max throughput of primitive restart, each triangle is replaced by one -1\n"
		"      (single per-scene vkCmdDrawIndexed() call,\n"
		"      no fragments produced):                  ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			cb.bindIndexBuffer(minusOneIndexBuffer.get(), 0, vk::IndexType::eUint32);
			beginTest(cb, attributelessConstantOutputPrimitiveRestartPipeline.get(),
			          simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			cb.drawIndexed(numTriangles, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   VS max throughput of primitive restart, only zeros in the index buffer\n"
		"      (single per-scene vkCmdDrawIndexed() call,\n"
		"      no fragments produced):                  ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			cb.bindIndexBuffer(zeroIndexBuffer.get(), 0, vk::IndexType::eUint32);
			beginTest(cb, attributelessConstantOutputPrimitiveRestartPipeline.get(),
			          simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			cb.drawIndexed(numTriangles+2, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	if(runAllTests) {

		tests.emplace_back(
			"   VS max throughput for triangle list, VertexIndex and InstanceIndex used for\n"
			"      position output (single per-scene vkCmdDraw() call,\n"
			"      attributeless:                           ",
			Test::Type::VertexThroughput,
			[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
			{
				beginTest(cb, attributelessInputIndicesPipeline.get(), simplePipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>(),
				          vector<vk::DescriptorSet>());
				cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
				endTest(cb, timestampIndex);
			}
		);

		tests.emplace_back(
			"   VS max throughput for indexed triangle list, VertexIndex and InstanceIndex\n"
			"      used for position output (single per-scene vkCmdDrawIndexed() call,\n"
			"      monotonically increasing indices,\n"
			"      attributeless, constant VS output):      ",
			Test::Type::VertexThroughput,
			[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
			{
				cb.bindIndexBuffer(indexBuffer.get(), 0, vk::IndexType::eUint32);
				beginTest(cb, attributelessInputIndicesPipeline.get(), simplePipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>(),
				          vector<vk::DescriptorSet>());
				cb.drawIndexed(3*numTriangles, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
				endTest(cb, timestampIndex);
			}
		);

		tests.emplace_back(
			"   VS max throughput for indexed triangle list that reuse two indices from\n"
			"      the previous triangle, VertexIndex and InstanceIndex used for\n"
			"      position output (single per-scene vkCmdDrawIndexed() call,\n"
			"      monotonically increasing indices,\n"
			"      attributeless, constant VS output):      ",
			Test::Type::VertexThroughput,
			[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
			{
				cb.bindIndexBuffer(stripIndexBuffer.get(), 0, vk::IndexType::eUint32);
				beginTest(cb, attributelessConstantOutputPipeline.get(), simplePipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>(),
				          vector<vk::DescriptorSet>());
				cb.drawIndexed(3*numTriangles, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
				endTest(cb, timestampIndex);
			}
		);

		tests.emplace_back(
			"   VS max throughput for triangle strip, VertexIndex and InstanceIndex\n"
			"      used for position output (per-strip vkCmdDraw() call, 1000 triangles\n"
			"      per strip, attributeless):               ",
			Test::Type::VertexThroughput,
			[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
			{
				beginTest(cb, attributelessInputIndicesTriStripPipeline.get(), simplePipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>(),
				          vector<vk::DescriptorSet>());
				for(uint32_t i=0,e=(numTriangles/maxTriStripLength)*(2+maxTriStripLength); i<e; i+=2+maxTriStripLength)
					cb.draw(2+maxTriStripLength, 1, i, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
				endTest(cb, timestampIndex);
			}
		);

		tests.emplace_back(
			"   VS max throughput for indexed triangle strip, VertexIndex and InstanceIndex\n"
			"      used for position output (per-strip vkCmdDrawIndexed() call,\n"
			"      1000 triangles per strip, monotonically increasing indices,\n"
			"      attributeless):                          ",
			Test::Type::VertexThroughput,
			[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
			{
				cb.bindIndexBuffer(indexBuffer.get(), 0, vk::IndexType::eUint32);
				beginTest(cb, attributelessInputIndicesTriStripPipeline.get(), simplePipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>(),
				          vector<vk::DescriptorSet>());
				for(uint32_t i=0,e=(numTriangles/maxTriStripLength)*(2+maxTriStripLength); i<e; i+=2+maxTriStripLength)
					cb.drawIndexed(2+maxTriStripLength, 1, i, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
				endTest(cb, timestampIndex);
			}
		);

		tests.emplace_back(
			"   VS max throughput for primitive restart indexed triangle strip,\n"
			"      VertexIndex and InstanceIndex used for position output\n"
			"      (single per-scene vkCmdDrawIndexed() call, 1000 triangles per strip\n"
			"      followed by -1, monotonically increasing indices,\n"
			"      attributeless):                          ",
			Test::Type::VertexThroughput,
			[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
			{
				cb.bindIndexBuffer(stripPrimitiveRestart1002IndexBuffer.get(), 0, vk::IndexType::eUint32);
				beginTest(cb, attributelessInputIndicesPrimitiveRestartPipeline.get(), simplePipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>(),
				          vector<vk::DescriptorSet>());
				uint32_t numIndicesPerStrip = maxTriStripLength+3;
				uint32_t numStrips = numTriangles/maxTriStripLength;
				cb.drawIndexed(numIndicesPerStrip*numStrips, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
				endTest(cb, timestampIndex);
			}
		);

	}

	tests.emplace_back(
		"   GS max throughput when no output is produced\n"
		"      (one draw call, attributeless):          ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			if(enabledFeatures.geometryShader) {
				beginTest(cb, geometryShaderNoOutputPipeline.get(), simplePipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>(),
				          vector<vk::DescriptorSet>());
				cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
				endTest(cb, timestampIndex);
			}
			else {
				tests[timestampIndex/2].enabled = false;
				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eTopOfPipe,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);
				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eColorAttachmentOutput,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);
			}
		}
	);

	tests.emplace_back(
		"   GS max throughput when single constant triangle is produced\n"
		"      (one draw call, attributeless):          ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			if(enabledFeatures.geometryShader) {
				beginTest(cb, geometryShaderConstantOutputPipeline.get(), simplePipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>(),
				          vector<vk::DescriptorSet>());
				cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
				endTest(cb, timestampIndex);
			}
			else {
				tests[timestampIndex/2].enabled = false;
				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eTopOfPipe,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);
				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eColorAttachmentOutput,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);
			}
		}
	);

	tests.emplace_back(
		"   GS max throughput when two constant triangles are produced\n"
		"      (one draw call, attributeless):          ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			if(enabledFeatures.geometryShader) {
				beginTest(cb, geometryShaderConstantOutputTwoTrianglesPipeline.get(),
				          simplePipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>(),
				          vector<vk::DescriptorSet>());
				cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
				endTest(cb, timestampIndex);
			}
			else {
				tests[timestampIndex/2].enabled = false;
				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eTopOfPipe,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);
				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eColorAttachmentOutput,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);
			}
		}
	);

	tests.emplace_back(
		"   Instancing throughput of vkCmdDraw()\n"
		"      (one triangle per instance, constant VS output, one draw call,\n"
		"      attributeless):                          ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, attributelessConstantOutputPipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			cb.draw(3, numTriangles, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Instancing throughput of vkCmdDrawIndexed()\n"
		"      (one triangle per instance, constant VS output, one draw call,\n"
		"      attributeless):                          ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			cb.bindIndexBuffer(indexBuffer.get(), 0, vk::IndexType::eUint32);
			beginTest(cb, attributelessConstantOutputPipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			cb.drawIndexed(3, numTriangles, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Instancing throughput of vkCmdDrawIndirect()\n"
		"      (one triangle per instance, one indirect draw call,\n"
		"      one indirect record, attributeless:      ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, attributelessConstantOutputPipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			cb.drawIndirect(indirectBuffer.get(),  // buffer
			                size_t(numTriangles)*sizeof(vk::DrawIndirectCommand),  // offset
			                1,  // drawCount
			                sizeof(vk::DrawIndirectCommand));  // stride
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Instancing throughput of vkCmdDrawIndexedIndirect()\n"
		"      (one triangle per instance, one indirect draw call,\n"
		"      one indirect record, attributeless:      ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			cb.bindIndexBuffer(indexBuffer.get(), 0, vk::IndexType::eUint32);
			beginTest(cb, attributelessConstantOutputPipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			cb.drawIndexedIndirect(indirectIndexedBuffer.get(),  // buffer
			                       size_t(numTriangles)*sizeof(vk::DrawIndexedIndirectCommand),  // offset
			                       1,  // drawCount
			                       sizeof(vk::DrawIndexedIndirectCommand));  // stride
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Draw command throughput\n"
		"      (per-triangle vkCmdDraw() in command buffer,\n"
		"      attributeless, constant VS output):      ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, attributelessConstantOutputPipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			for(uint32_t i=0; i<numTriangles; i++)
				cb.draw(3, 1, i*3, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Draw indexed command throughput\n"
		"      (per-triangle vkCmdDrawIndexed() in command buffer,\n"
		"      attributeless, constant VS output):      ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			cb.bindIndexBuffer(indexBuffer.get(), 0, vk::IndexType::eUint32);
			beginTest(cb, attributelessConstantOutputPipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			for(uint32_t i=0; i<numTriangles; i++)
				cb.drawIndexed(3, 1, i*3, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
			endTest(cb, timestampIndex);
		}
	);

#if 0 // the tests are probably not needed
	tests.emplace_back(
		"   Draw command throughput with vec4 attribute\n"
		"      (per-triangle vkCmdDraw() in command buffer,\n"
		"      vec4 coordinate attribute):              ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, coordinateAttributePipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ coordinate4Attribute.get() },
			          vector<vk::DescriptorSet>());
			for(uint32_t i=0; i<numTriangles; i++)
				cb.draw(3, 1, i*3, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Draw indexed command throughput with vec4 attribute\n"
		"      (per-triangle vkCmdDrawIndexed() in command buffer,\n"
		"      vec4 coordinate attribute):              ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			cb.bindIndexBuffer(indexBuffer.get(), 0, vk::IndexType::eUint32);
			beginTest(cb, coordinateAttributePipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ coordinate4Attribute.get() },
			          vector<vk::DescriptorSet>());
			for(uint32_t i=0; i<numTriangles; i++)
				cb.drawIndexed(3, 1, i*3, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
			endTest(cb, timestampIndex);
		}
	);

#endif

	tests.emplace_back(
		"   VkDrawIndirectCommand processing throughput\n"
		"      (per-triangle VkDrawIndirectCommand, one vkCmdDrawIndirect() call,\n"
		"      attributeless):                          ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, attributelessConstantOutputPipeline.get(),
			          simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			if(enabledFeatures.multiDrawIndirect)
				cb.drawIndirect(indirectBuffer.get(),  // buffer
				                0,  // offset
				                numTriangles,  // drawCount
				                sizeof(vk::DrawIndirectCommand));  // stride
			else
				tests[timestampIndex/2].enabled = false;
			endTest(cb, timestampIndex);
		}
	);

#if 0 // the test is probably not needed
	tests.emplace_back(
		"   VkDrawIndirectCommand processing throughput with vec4 attribute\n"
		"      (per-triangle VkDrawIndirectCommand, one vkCmdDrawIndirect() call,\n"
		"      vec4 coordiate attribute):               ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, coordinateAttributePipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ coordinate4Attribute.get() },
			          vector<vk::DescriptorSet>());
			if(enabledFeatures.multiDrawIndirect)
				cb.drawIndirect(indirectBuffer.get(),  // buffer
				                0,  // offset
				                numTriangles,  // drawCount
				                sizeof(vk::DrawIndirectCommand));  // stride
			else
				tests[timestampIndex/2].enabled = false;
			endTest(cb, timestampIndex);
		}
	);
#endif

	tests.emplace_back(
		("   VkDrawIndirectCommand processing throughput with stride " + to_string(indirectRecordStride) + "\n"
		"      (per-triangle VkDrawIndirectCommand, one vkCmdDrawIndirect() call,\n"
		"      attributeless):                          ").c_str(),
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, attributelessConstantOutputPipeline.get(),
			          simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			if(enabledFeatures.multiDrawIndirect)
				cb.drawIndirect(indirectStrideBuffer.get(),  // buffer
				                0,  // offset
				                numTriangles,  // drawCount
				                indirectRecordStride);  // stride
			else
				tests[timestampIndex/2].enabled = false;
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   VkDrawIndexedIndirectCommand processing throughput\n"
		"      (per-triangle VkDrawIndexedIndirectCommand, 1x vkCmdDrawIndexedIndirect()\n"
		"      call, attributeless):                    ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			cb.bindIndexBuffer(indexBuffer.get(), 0, vk::IndexType::eUint32);
			beginTest(cb, attributelessConstantOutputPipeline.get(),
			          simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			if(enabledFeatures.multiDrawIndirect)
				cb.drawIndexedIndirect(indirectIndexedBuffer.get(),  // buffer
				                       0,  // offset
				                       numTriangles,  // drawCount
				                       sizeof(vk::DrawIndexedIndirectCommand));  // stride
			else
				tests[timestampIndex/2].enabled = false;
			endTest(cb, timestampIndex);
		}
	);

#if 0 // the test is probably not needed
	tests.emplace_back(
		"   VkDrawIndexedIndirectCommand processing throughput with vec4 attribute\n"
		"      (per-triangle VkDrawIndexedIndirectCommand, 1x vkCmdDrawIndexedIndirect()\n"
		"      call, vec4 coordiate attribute):         ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			cb.bindIndexBuffer(indexBuffer.get(), 0, vk::IndexType::eUint32);
			beginTest(cb, coordinateAttributePipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ coordinate4Attribute.get() },
			          vector<vk::DescriptorSet>());
			if(enabledFeatures.multiDrawIndirect)
				cb.drawIndexedIndirect(indirectIndexedBuffer.get(),  // buffer
				                       0,  // offset
				                       numTriangles,  // drawCount
				                       sizeof(vk::DrawIndexedIndirectCommand));  // stride
			else
				tests[timestampIndex/2].enabled = false;
			endTest(cb, timestampIndex);
		}
	);
#endif

	tests.emplace_back(
		("   VkDrawIndexedIndirectCommand processing throughput with stride " + to_string(indirectRecordStride) + "\n"
		"      (per-triangle VkDrawIndexedIndirectCommand, 1x vkCmdDrawIndexedIndirect()\n"
		"      call, attributeless):                    ").c_str(),
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			cb.bindIndexBuffer(indexBuffer.get(), 0, vk::IndexType::eUint32);
			beginTest(cb, attributelessConstantOutputPipeline.get(),
			          simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			if(enabledFeatures.multiDrawIndirect)
				cb.drawIndexedIndirect(indirectIndexedStrideBuffer.get(),  // buffer
				                       0,  // offset
				                       numTriangles,  // drawCount
				                       indirectRecordStride);  // stride
			else
				tests[timestampIndex/2].enabled = false;
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   One attribute performance - 1x vec4 attribute\n"
		"      (attribute used, one draw call):         ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, coordinateAttributePipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ coordinate4Attribute.get() },
			          vector<vk::DescriptorSet>());
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   One buffer performance - 1x vec4 buffer\n"
		"      (1x read in VS, one draw call):          ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, coordinate4BufferPipeline.get(), oneBufferPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>{ coordinate4BufferDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   One buffer performance - 1x vec3 buffer\n"
		"      (1x read in VS, one draw call):          ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, coordinate3BufferPipeline.get(), oneBufferPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>{ coordinate3BufferDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Two attributes performance - 2x vec4 attribute\n"
		"      (both attributes used):                  ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, twoAttributesPipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ coordinate4Attribute.get(), vec4Attributes[0].get() },
			          vector<vk::DescriptorSet>());
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Two buffers performance - 2x vec4 buffer\n"
		"      (both buffers read in VS):               ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, twoBuffersPipeline.get(), twoBuffersPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>{ twoBuffersDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Two buffers performance - 2x vec3 buffer\n"
		"      (both buffers read in VS):               ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, twoBuffer3Pipeline.get(), twoBuffersPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>{ twoBuffer3DescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Two interleaved attributes performance - 2x vec4\n"
		"      (2x vec4 attribute fetched from the single buffer in VS\n"
		"      from consecutive buffer locations:       ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, twoInterleavedAttributesPipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ twoInterleavedAttributes.get() },
			          vector<vk::DescriptorSet>());
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);
			
	tests.emplace_back(
		"   Two interleaved buffers performance - 2x vec4\n"
		"      (2x vec4 fetched from the single buffer in VS\n"
		"      from consecutive buffer locations:       ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, twoInterleavedBuffersPipeline.get(), oneBufferPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>{ twoInterleavedBuffersDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Packed buffer performance - 1x buffer using 32-byte struct unpacked\n"
		"      into position+normal+color+texCoord:     ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, singlePackedBufferPipeline.get(), oneBufferPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>{ singlePackedBufferDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Packed attribute performance - 2x uvec4 attribute unpacked\n"
		"      into position+normal+color+texCoord:     ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, twoPackedAttributesPipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
			          vector<vk::DescriptorSet>());
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Packed buffer performance - 2x uvec4 buffers unpacked\n"
		"      into position+normal+color+texCoord:     ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, twoPackedBuffersPipeline.get(), twoBuffersPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>{ twoPackedBuffersDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Packed buffer performance - 2x buffer using 16-byte struct unpacked\n"
		"      into position+normal+color+texCoord:     ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, twoPackedBuffersUsingStructPipeline.get(), twoBuffersPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>{ twoPackedBuffersDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Packed buffer performance - 2x buffer using 16-byte struct\n"
		"      read multiple times and unpacked\n"
		"      into position+normal+color+texCoord:     ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, twoPackedBuffersUsingStructSlowPipeline.get(), twoBuffersPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>{ twoPackedBuffersDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Four attributes performance - 4x vec4 attribute\n"
		"      (all attributes used):                   ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, fourAttributesPipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ coordinate4Attribute.get(), vec4Attributes[0].get(),
			                              vec4Attributes[1].get(), vec4Attributes[2].get() },
			          vector<vk::DescriptorSet>());
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Four buffers performance - 4x vec4 buffer\n"
		"      (all buffers read in VS):                ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, fourBuffersPipeline.get(), fourBuffersPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>{ fourBuffersDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Four buffers performance - 4x vec3 buffer\n"
		"      (all buffers read in VS):                ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, fourBuffer3Pipeline.get(), fourBuffersPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>{ fourBuffer3DescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Four interleaved attributes performance - 4x vec4\n"
		"      (4x vec4 fetched from the single buffer\n"
		"      on consecutive locations:                ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, fourInterleavedAttributesPipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ fourInterleavedAttributes.get() },
			          vector<vk::DescriptorSet>());
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Four interleaved buffers performance - 4x vec4\n"
		"      (4x vec4 fetched from the single buffer\n"
		"      on consecutive locations:                ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, fourInterleavedBuffersPipeline.get(), oneBufferPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>{ fourInterleavedBuffersDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Four attributes performance - 2x vec4 and 2x uint attribute\n"
		"      (2x vec4f32 + 2x vec4u8, 2x conversion from vec4u8\n"
		"      to vec4):                                ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, two4F32Two4U8AttributesPipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ coordinate4Attribute.get(), vec4Attributes[0].get(),
			                              vec4u8Attributes[0].get(), vec4u8Attributes[1].get() },
			          vector<vk::DescriptorSet>());
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Matrix performance - one matrix as uniform for all triangles\n"
		"      (maxtrix read in VS,\n"
		"      coordinates in vec4 attribute):          ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, singleMatrixUniformPipeline.get(), oneUniformVSPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ coordinate4Attribute.get() },
			          vector<vk::DescriptorSet>{ oneUniformVSDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Matrix performance - per-triangle matrix in buffer\n"
		"      (different matrix read for each triangle in VS,\n"
		"      coordinates in vec4 attribute):          ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, matrixBufferPipeline.get(), oneBufferPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ coordinate4Attribute.get() },
			          vector<vk::DescriptorSet>{ sameMatrixBufferDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Matrix performance - per-triangle matrix in attribute\n"
		"      (triangles are instanced and each triangle receives a different matrix,\n"
		"      coordinates in vec4 attribute:           ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, matrixAttributePipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ coordinate4Attribute.get(), transformationMatrixAttribute.get() },
			          vector<vk::DescriptorSet>());
			cb.draw(3, numTriangles/2, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			cb.draw(3, numTriangles/2, 3, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Matrix performance - one matrix in buffer for all triangles and 2x uvec4\n"
		"      packed attributes (each triangle reads matrix from the same place in\n"
		"      the buffer, attributes unpacked):        ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, twoPackedAttributesAndSingleMatrixPipeline.get(),
			          oneBufferPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
			          vector<vk::DescriptorSet>{ sameMatrixBufferDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Matrix performance - per-triangle matrix in the buffer and 2x uvec4 packed\n"
		"      attributes (each triangle reads a different matrix from a buffer,\n"
		"      attributes unpacked):                    ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, twoPackedAttributesAndMatrixPipeline.get(), oneBufferPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
			          vector<vk::DescriptorSet>{ sameMatrixBufferDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Matrix performance - per-triangle matrix in buffer and 2x uvec4 packed\n"
		"      buffers (each triangle reads a different matrix from a buffer,\n"
		"      packed buffers unpacked):                ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, twoPackedBuffersAndMatrixPipeline.get(), threeBuffersPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>{ threeBuffersDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Matrix performance - GS reads per-triangle matrix from buffer and 2x uvec4\n"
		"      packed buffers (each triangle reads a different matrix from a buffer,\n"
		"      packed buffers unpacked in GS):          ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			if(enabledFeatures.geometryShader) {
				beginTest(cb, geometryShaderPipeline.get(), threeBuffersInGSPipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>(),
				          vector<vk::DescriptorSet>{ threeBuffersInGSDescriptorSet });
				cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
				endTest(cb, timestampIndex);
			}
			else {
				tests[timestampIndex/2].enabled = false;
				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eTopOfPipe,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);
				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eColorAttachmentOutput,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);
			}
		}
	);

	tests.emplace_back(
		"   Matrix performance - per-triangle matrix in buffer and four attributes\n"
		"      (each triangle reads a different matrix from a buffer,\n"
		"      4x vec4 attribute):                      ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, fourAttributesAndMatrixPipeline.get(), oneBufferPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ coordinate4Attribute.get(), vec4Attributes[0].get(),
			                              vec4Attributes[1].get(), vec4Attributes[2].get() },
			          vector<vk::DescriptorSet>{ sameMatrixBufferDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Matrix performance - 1x per-triangle matrix in buffer, 2x uniform matrix and\n"
		"      and 2x uvec4 packed attributes (uniform view and projection matrices\n"
		"      multiplied with per-triangle model matrix and with unpacked attributes of\n"
		"      position, normal, color and texCoord:    ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, transformationThreeMatricesPipeline.get(),
			          bufferAndUniformPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
			          vector<vk::DescriptorSet>{ transformationThreeMatricesDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Matrix performance - 2x per-triangle matrix (mat4+mat3) in buffer,\n"
		"      3x uniform matrix (mat4+mat4+mat3) and 2x uvec4 packed attributes\n"
		"      (full position and normal computation with MVP and normal matrices,\n"
		"      all matrices and attributes multiplied): ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, transformationFiveMatricesPipeline.get(),
			          twoBuffersAndUniformPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
			          vector<vk::DescriptorSet>{ transformationFiveMatricesDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Matrix performance - 2x per-triangle matrix (mat4+mat3) in buffer,\n"
		"      2x non-changing matrix (mat4+mat4) in push constants,\n"
		"      1x constant matrix (mat3) and 2x uvec4 packed attributes (all\n"
		"      matrices and attributes multiplied):     ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			cb.pushConstants(
				twoBuffersAndPushConstantsPipelineLayout.get(),  // layout
				vk::ShaderStageFlagBits::eVertex,  // stageFlags
				0,  // offset
				128,  // size
				array<float,32>{  // pValues
					1.f,0.f,0.f,0.f,
					0.f,1.f,0.f,0.f,
					0.f,0.f,1.f,0.f,
					0.f,0.f,0.f,1.f,
					1.f,0.f,0.f,0.f,
					0.f,1.f,0.f,0.f,
					0.f,0.f,1.f,0.f,
					0.f,0.f,0.f,1.f,
				}.data()
			);
			beginTest(cb, transformationFiveMatricesPushConstantsPipeline.get(),
			          twoBuffersAndPushConstantsPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
			          vector<vk::DescriptorSet>{ transformationTwoMatricesDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Matrix performance - 2x per-triangle matrix (mat4+mat3) in buffer, 2x\n"
		"      non-changing matrix (mat4+mat4) in specialization constants, 1x constant\n"
		"      matrix (mat3) defined by VS code and 2x uvec4 packed attributes (all\n"
		"      matrices and attributes multiplied):     ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, transformationFiveMatricesSpecializationConstantsPipeline.get(),
			          twoBuffersPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
			          vector<vk::DescriptorSet>{ transformationTwoMatricesDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Matrix performance - 2x per-triangle matrix (mat4+mat3) in buffer,\n"
		"      3x constant matrix (mat4+mat4+mat3) defined by VS code and\n"
		"      2x uvec4 packed attributes (all matrices and attributes\n"
		"      multiplied):                             ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, transformationFiveMatricesConstantsPipeline.get(),
			          twoBuffersPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
			          vector<vk::DescriptorSet>{ transformationTwoMatricesDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Matrix performance - GS five matrices processing, 2x per-triangle matrix\n"
		"      (mat4+mat3) in buffer, 3x uniform matrix (mat4+mat4+mat3) and\n"
		"      2x uvec4 packed attributes passed through VS (all matrices and\n"
		"      attributes multiplied):                  ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			if(enabledFeatures.geometryShader) {
				beginTest(cb, transformationFiveMatricesUsingGSAndAttributesPipeline.get(),
				          twoBuffersAndUniformInGSPipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
				          vector<vk::DescriptorSet>{ transformationFiveMatricesUsingGSAndAttributesDescriptorSet });
				cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
				endTest(cb, timestampIndex);
			} else {
				tests[timestampIndex/2].enabled = false;
				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eTopOfPipe,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);
				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eColorAttachmentOutput,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);
			}
		}
	);

	tests.emplace_back(
		"   Matrix performance - GS five matrices processing, 2x per-triangle matrix\n"
		"      (mat4+mat3) in buffer, 3x uniform matrix (mat4+mat4+mat3) and\n"
		"      2x uvec4 packed data read from buffer in GS (all matrices and attributes\n"
		"      multiplied):                             ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			if(enabledFeatures.geometryShader) {
				beginTest(cb, transformationFiveMatricesUsingGSPipeline.get(),
				          fourBuffersAndUniformInGSPipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>(),
				          vector<vk::DescriptorSet>{ transformationFiveMatricesUsingGSDescriptorSet });
				cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
				endTest(cb, timestampIndex);
			} else {
				tests[timestampIndex/2].enabled = false;
				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eTopOfPipe,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);
				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eColorAttachmentOutput,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);
			}
		}
	);

	tests.emplace_back(
		"   Textured Phong and Matrix performance - 2x per-triangle matrix\n"
		"      in buffer (mat4+mat3), 3x uniform matrix (mat4+mat4+mat3) and\n"
		"      four attributes (vec4f32+vec3f32+vec4u8+vec2f32),\n"
		"      no fragments produced:                   ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, phongTexturedFourAttributesFiveMatricesPipeline.get(),
			          twoBuffersAndUniformPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ coordinate4Attribute.get(), normalAttribute.get(),
			                              colorAttribute.get(), texCoordAttribute.get() },
			          vector<vk::DescriptorSet>{ transformationFiveMatricesDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Textured Phong and Matrix performance - 1x per-triangle matrix\n"
		"      in buffer (mat4), 2x uniform matrix (mat4+mat4) and\n"
		"      four attributes (vec4f32+vec3f32+vec4u8+vec2f32),\n"
		"      no fragments produced:                   ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, phongTexturedFourAttributesPipeline.get(),
			          bufferAndUniformPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ coordinate4Attribute.get(), normalAttribute.get(),
			                              colorAttribute.get(), texCoordAttribute.get() },
			          vector<vk::DescriptorSet>{ transformationThreeMatricesDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Textured Phong and Matrix performance - 1x per-triangle matrix\n"
		"      in buffer (mat4), 2x uniform matrix (mat4+mat4) and 2x uvec4 packed\n"
		"      attribute, no fragments produced:        ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, phongTexturedPipeline.get(), bufferAndUniformPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
			          vector<vk::DescriptorSet>{ transformationThreeMatricesDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Textured Phong and Matrix performance - 1x per-triangle row-major matrix\n"
		"      in buffer (mat4), 2x uniform not-row-major matrix (mat4+mat4),\n"
		"      2x uvec4 packed attributes,\n"
		"      no fragments produced:                   ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, phongTexturedRowMajorPipeline.get(), bufferAndUniformPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
			          vector<vk::DescriptorSet>{ transformationThreeMatricesRowMajorDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Textured Phong and Matrix performance - 1x per-triangle mat4x3 matrix\n"
		"      in buffer, 2x uniform matrix (mat4+mat4) and 2x uvec4 packed attributes,\n"
		"      no fragments produced:                   ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, phongTexturedMat4x3Pipeline.get(), bufferAndUniformPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
			          vector<vk::DescriptorSet>{ transformationThreeMatrices4x3DescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Textured Phong and Matrix performance - 1x per-triangle row-major mat4x3\n"
		"      matrix in buffer, 2x uniform matrix (mat4+mat4), 2x uvec4 packed\n"
		"      attribute, no fragments produced:        ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, phongTexturedMat4x3RowMajorPipeline.get(),
			          bufferAndUniformPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
			          vector<vk::DescriptorSet>{ transformationThreeMatrices4x3RowMajorDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Textured Phong and PAT performance - PAT v1 (Position-Attitude-Transform,\n"
		"      performing translation (vec3) and rotation (quaternion as vec4) using\n"
		"      implementation 1), PAT is per-triangle 2x vec4 in buffer,\n"
		"      2x uniform matrix (mat4+mat4), 2x uvec4 packed attributes,\n"
		"      no fragments produced:                   ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, phongTexturedQuat1Pipeline.get(), bufferAndUniformPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
			          vector<vk::DescriptorSet>{ transformationTwoMatricesAndPATDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Textured Phong and PAT performance - PAT v2 (Position-Attitude-Transform,\n"
		"      performing translation (vec3) and rotation (quaternion as vec4) using\n"
		"      implementation 2), PAT is per-triangle 2x vec4 in buffer,\n"
		"      2x uniform matrix (mat4+mat4), 2x uvec4 packed attributes,\n"
		"      no fragments produced:                   ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, phongTexturedQuat2Pipeline.get(), bufferAndUniformPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
			          vector<vk::DescriptorSet>{ transformationTwoMatricesAndPATDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Textured Phong and PAT performance - PAT v3 (Position-Attitude-Transform,\n"
		"      performing translation (vec3) and rotation (quaternion as vec4) using\n"
		"      implementation 3), PAT is per-triangle 2x vec4 in buffer,\n"
		"      2x uniform matrix (mat4+mat4), 2x uvec4 packed attributes,\n"
		"      no fragments produced:                   ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, phongTexturedQuat3Pipeline.get(), bufferAndUniformPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
			          vector<vk::DescriptorSet>{ transformationTwoMatricesAndPATDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Textured Phong and PAT performance - constant single PAT v2 sourced from\n"
		"      the same index in buffer (vec3+vec4), 2x uniform matrix (mat4+mat4),\n"
		"      2x uvec4 packed attributes,\n"
		"      no fragments produced:                   ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			beginTest(cb, phongTexturedSingleQuat2Pipeline.get(), bufferAndUniformPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
			          vector<vk::DescriptorSet>{ transformationTwoMatricesAndSinglePATDescriptorSet });
			cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Textured Phong and PAT performance - indexed draw call, per-triangle PAT v2\n"
		"      in buffer (vec3+vec4), 2x uniform matrix (mat4+mat4), 2x uvec4 packed\n"
		"      attribute, no fragments produced:        ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			cb.bindIndexBuffer(indexBuffer.get(), 0, vk::IndexType::eUint32);
			beginTest(cb, phongTexturedQuat2Pipeline.get(), bufferAndUniformPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
			          vector<vk::DescriptorSet>{ transformationTwoMatricesAndPATDescriptorSet });
			cb.drawIndexed(3*numTriangles, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Textured Phong and PAT performance - indexed draw call, constant single\n"
		"      PAT v2 sourced from the same index in buffer (vec3+vec4),\n"
		"      2x uniform matrix (mat4+mat4), 2x uvec4 packed attributes,\n"
		"      no fragments produced:                   ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			cb.bindIndexBuffer(indexBuffer.get(), 0, vk::IndexType::eUint32);
			beginTest(cb, phongTexturedSingleQuat2Pipeline.get(),
			          bufferAndUniformPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
			          vector<vk::DescriptorSet>{ transformationTwoMatricesAndSinglePATDescriptorSet });
			cb.drawIndexed(3*numTriangles, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Textured Phong and PAT performance - primitive restart, indexed draw call,\n"
		"      per-triangle PAT v2 in buffer (vec3+vec4), 2x uniform matrix (mat4+mat4),\n"
		"      2x uvec4 packed attributes,\n"
		"      no fragments produced:                   ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			cb.bindIndexBuffer(primitiveRestartIndexBuffer.get(), 0, vk::IndexType::eUint32);
			beginTest(cb, phongTexturedQuat2PrimitiveRestartPipeline.get(),
			          bufferAndUniformPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
			          vector<vk::DescriptorSet>{ transformationTwoMatricesAndPATDescriptorSet });
			cb.drawIndexed(4*numTriangles, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Textured Phong and PAT performance - primitive restart, indexed draw call,\n"
		"      constant single PAT v2 sourced from the same index in buffer (vec3+vec4),\n"
		"      2x uniform matrix (mat4+mat4), 2x uvec4 packed attributes,\n"
		"      no fragments produced:                   ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			cb.bindIndexBuffer(primitiveRestartIndexBuffer.get(), 0, vk::IndexType::eUint32);
			beginTest(cb, phongTexturedSingleQuat2PrimitiveRestartPipeline.get(),
			          bufferAndUniformPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
			          vector<vk::DescriptorSet>{ transformationTwoMatricesAndSinglePATDescriptorSet });
			cb.drawIndexed(4*numTriangles, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Textured Phong and double precision matrix performance - double precision\n"
		"      per-triangle matrix in buffer (dmat4), double precision per-scene view\n"
		"      matrix in uniform (dmat4), both matrices converted to single precision\n"
		"      before computations, single precision per-scene perspective matrix in\n"
		"      uniform (mat4), single precision vertex positions, packed attributes\n"
		"      (2x uvec4), no fragments produced:       ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			if(enabledFeatures.shaderFloat64) {
				beginTest(cb, phongTexturedDMatricesOnlyInputPipeline.get(),
				          bufferAndUniformPipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
				          vector<vk::DescriptorSet>{ transformationThreeDMatricesDescriptorSet });
				cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
				endTest(cb, timestampIndex);
			} else {
				tests[timestampIndex/2].enabled = false;
				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eTopOfPipe,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);
				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eColorAttachmentOutput,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);
			}
		}
	);

	tests.emplace_back(
		"   Textured Phong and double precision matrix performance - double precision\n"
		"      per-triangle matrix in buffer (dmat4), double precision per-scene view\n"
		"      matrix in uniform (dmat4), both matrices multiplied in double precision,\n"
		"      single precision vertex positions, single precision per-scene\n"
		"      perspective matrix in uniform (mat4), packed attributes (2x uvec4),\n"
		"      no fragments produced:                   ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			if(enabledFeatures.shaderFloat64) {
				beginTest(cb, phongTexturedDMatricesPipeline.get(),
				          bufferAndUniformPipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>{ packedAttribute1.get(), packedAttribute2.get() },
				          vector<vk::DescriptorSet>{ transformationThreeDMatricesDescriptorSet });
				cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
				endTest(cb, timestampIndex);
			} else {
				tests[timestampIndex/2].enabled = false;
				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eTopOfPipe,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);
				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eColorAttachmentOutput,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);
			}
		}
	);

	tests.emplace_back(
		"   Textured Phong and double precision matrix performance - double precision\n"
		"      per-triangle matrix in buffer (dmat4), double precision per-scene view\n"
		"      matrix in uniform (dmat4), both matrices multiplied in double precision,\n"
		"      double precision vertex positions (dvec3), single precision per-scene\n"
		"      perspective matrix in uniform (mat4), packed attributes (3x uvec4),\n"
		"      no fragments produced:                   ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			if(enabledFeatures.shaderFloat64) {
				beginTest(cb, phongTexturedDMatricesDVerticesPipeline.get(),
				          bufferAndUniformPipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>{ packedDAttribute1.get(), packedDAttribute2.get(), packedDAttribute3.get() },
				          vector<vk::DescriptorSet>{ transformationThreeDMatricesDescriptorSet });
				cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
				endTest(cb, timestampIndex);
			} else {
				tests[timestampIndex/2].enabled = false;
				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eTopOfPipe,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);
				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eColorAttachmentOutput,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);
			}
		}
	);

	tests.emplace_back(
		"   Textured Phong and double precision matrix performance using GS - double\n"
		"      precision per-triangle matrix in buffer (dmat4), double precision\n"
		"      per-scene view matrix in uniform (dmat4), both matrices multiplied in\n"
		"      double precision, double precision vertex positions (dvec3), single\n"
		"      precision per-scene perspective matrix in uniform (mat4), packed\n"
		"      attributes (3x uvec4),\n"
		"      no fragments produced:                   ",
		Test::Type::VertexThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			if(enabledFeatures.shaderFloat64 && enabledFeatures.geometryShader) {
				beginTest(cb, phongTexturedInGSDMatricesDVerticesPipeline.get(),
				          bufferAndUniformInGSPipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>{ packedDAttribute1.get(), packedDAttribute2.get(), packedDAttribute3.get() },
				          vector<vk::DescriptorSet>{ phongTexturedThreeDMatricesUsingGSAndAttributesDescriptorSet });
				cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
				endTest(cb, timestampIndex);
			} else {
				tests[timestampIndex/2].enabled = false;
				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eTopOfPipe,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);
				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eColorAttachmentOutput,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);
			}
		}
	);

	if(runAllTests) {

		const char* sharedVertexPerformanceText =
			"   Shared vertex performance - strip-like geometry of various lengths,\n"
			"      per-strip vkCmdDraw() call, each next triangle duplicates\n"
			"      two vertices of previous triangle, textured Phong,\n"
			"      per-scene PAT v2 (Position-Attitude-Transform: translation (vec3)+\n"
			"      quaternion rotation (vec4)) read from the same index in the buffer,\n"
			"      2x uniform matrix (mat4+mat4), packed attributes (2x uvec4),\n"
			"      simplified FS that just uses all inputs: ";
		for(uint32_t n : array<uint32_t,12>{1,2,5,8,10,20,25,40,50,100,125,maxTriStripLength}) {  // numbers in this list needs to be divisible by maxTriStripLength (1000 by default) with reminder zero

			tests.emplace_back(
				sharedVertexPerformanceText,
				n,
				[](uint32_t n) {
					string s = static_cast<stringstream&&>(stringstream() << "         strip length " << n << ": ").str();
					s.append(28-s.size(), ' ');
					return s;
				}(n).c_str(),
				Test::Type::VertexThroughput,
				[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t n)
				{
					beginTest(cb, phongTexturedSingleQuat2Pipeline.get(),
					          bufferAndUniformPipelineLayout.get(), timestampIndex,
					          vector<vk::Buffer>{ sharedVertexPackedAttribute1.get(), sharedVertexPackedAttribute2.get() },
					          vector<vk::DescriptorSet>{ transformationTwoMatricesAndSinglePATDescriptorSet });
					for(uint32_t i=0,e=3*numTriangles; i<e; i+=n*3)
						cb.draw(min(n*3, e-i), 1, i, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
					endTest(cb, timestampIndex);
				}
			);

		}

		const char* indexedSharedVertexPerformanceText =
			"   Indexed shared vertex performance - strip-like geometry of various\n"
			"      lengths, per-strip vkCmdDrawIndexed() call, indices of the next\n"
			"      triangle reuse two vertices of previous triangle, textured Phong,\n"
			"      constant single PAT v2 (vec3+vec4) read from the same index\n"
			"      in the buffer, 2x uniform matrix (mat4+mat4),\n"
			"      packed attributes (2x uvec4)\n"
			"      simplified FS that just uses all inputs: ";
		for(uint32_t n : array<uint32_t,12>{1,2,5,8,10,20,25,40,50,100,125,maxTriStripLength}) {  // numbers in this list needs to be divisible by maxTriStripLength (1000 by default) with reminder zero

			tests.emplace_back(
				indexedSharedVertexPerformanceText,
				n,
				[](uint32_t n) {
					string s = static_cast<stringstream&&>(stringstream() << "         strip length " << n << ": ").str();
					s.append(28-s.size(), ' ');
					return s;
				}(n).c_str(),
				Test::Type::VertexThroughput,
				[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t n)
				{
					cb.bindIndexBuffer(stripIndexBuffer.get(), 0, vk::IndexType::eUint32);
					beginTest(cb, phongTexturedSingleQuat2Pipeline.get(),
					          bufferAndUniformPipelineLayout.get(), timestampIndex,
					          vector<vk::Buffer>{ stripPackedAttribute1.get(), stripPackedAttribute2.get() },
					          vector<vk::DescriptorSet>{ transformationTwoMatricesAndSinglePATDescriptorSet });
					for(uint32_t i=0,e=3*numTriangles; i<e; i+=n*3)
						cb.drawIndexed(min(n*3, e-i), 1, i, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
					endTest(cb, timestampIndex);
				}
			);
		}

		const char* triStripSingleQuat2PerformanceText =
			"   Triangle strip performance - strips of various lengths,\n"
			"      per-strip vkCmdDraw() call, textured Phong, constant single\n"
			"      PAT v2 (vec3+vec4) read from the same index in the buffer,\n"
			"      2x uniform matrix (mat4+mat4), packed attributes (2x uvec4),\n"
			"      simplified FS that just uses all inputs: ";
		for(uint32_t n : array<uint32_t,12>{1,2,5,8,10,20,25,40,50,100,125,maxTriStripLength}) {  // numbers in this list needs to be divisible by maxTriStripLength (1000 by default) with reminder zero

			tests.emplace_back(
				triStripSingleQuat2PerformanceText,
				n,
				[](uint32_t n) {
					string s = static_cast<stringstream&&>(stringstream() << "         strip length " << n << ": ").str();
					s.append(28-s.size(), ' ');
					return s;
				}(n).c_str(),
				Test::Type::VertexThroughput,
				[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t n)
				{
					beginTest(cb, phongTexturedSingleQuat2TriStripPipeline.get(),
					          bufferAndUniformPipelineLayout.get(), timestampIndex,
					          vector<vk::Buffer>{ stripPackedAttribute1.get(), stripPackedAttribute2.get() },
					          vector<vk::DescriptorSet>{ transformationTwoMatricesAndSinglePATDescriptorSet });
					for(uint32_t i=0,e=(numTriangles/maxTriStripLength)*(2+maxTriStripLength); i<e; i+=2+maxTriStripLength)
						for(uint32_t j=i,je=i+maxTriStripLength; j<je; j+=n)
							cb.draw(n+2, 1, j, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
					endTest(cb, timestampIndex);
				}
			);
		}

		const char* indexedTriStripSingleQuat2PerformanceText =
			"   Indexed triangle strip performance - strips of various lengths,\n"
			"      per-strip vkCmdDrawIndexed() call, indices just increase for each\n"
			"      next vertex, textured Phong, constant single PAT v2 (vec3+vec4) read\n"
			"      from the same index in the buffer, 2x uniform matrix (mat4+mat4),\n"
			"      packed attributes (2x uvec4),\n"
			"      simplified FS that just uses all inputs: ";
		for(uint32_t n : array<uint32_t,12>{1,2,5,8,10,20,25,40,50,100,125,maxTriStripLength}) {  // numbers in this list needs to be divisible by maxTriStripLength (1000 by default) with reminder zero

			tests.emplace_back(
				indexedTriStripSingleQuat2PerformanceText,
				n,
				[](uint32_t n) {
					string s = static_cast<stringstream&&>(stringstream() << "         strip length " << n << ": ").str();
					s.append(28-s.size(), ' ');
					return s;
				}(n).c_str(),
				Test::Type::VertexThroughput,
				[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t n)
				{
					cb.bindIndexBuffer(indexBuffer.get(), 0, vk::IndexType::eUint32);
					beginTest(cb, phongTexturedSingleQuat2TriStripPipeline.get(),
					          bufferAndUniformPipelineLayout.get(), timestampIndex,
					          vector<vk::Buffer>{ stripPackedAttribute1.get(), stripPackedAttribute2.get() },
					          vector<vk::DescriptorSet>{ transformationTwoMatricesAndSinglePATDescriptorSet });
					for(uint32_t i=0,e=(numTriangles/maxTriStripLength)*(2+maxTriStripLength); i<e; i+=2+maxTriStripLength)
						for(uint32_t j=i,je=i+maxTriStripLength; j<je; j+=n)
							cb.drawIndexed(n+2, 1, j, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
					endTest(cb, timestampIndex);
				}
			);
		}

		const char* primitiveRestartSingleQuat2PerformanceText =
			"   Primitive restart performance with single per-scene vkCmdDrawIndexed() call,\n"
			"      strips of various lengths (1-1000 triangles) each finished by -1,\n"
			"      rendering as triangle strips, textured Phong, constant single\n"
			"      PAT v2 (vec3+vec4) read from the same index in the buffer,\n"
			"      2x uniform matrix (mat4+mat4), packed attributes (2x uvec4),\n"
			"      no fragments produced:                   ";
		for(uint32_t n : array<uint32_t,5>{1,2,5,8,1000}) {  // numbers in this list needs to be divisible by maxTriStripLength (1000 by default) with reminder zero

			tests.emplace_back(
				primitiveRestartSingleQuat2PerformanceText,
				n,
				[](uint32_t n) {
					string s = static_cast<stringstream&&>(stringstream()
					           << "         strip length " << n << ":   ").str();
					if(s.size()<28)
						s.append(28-s.size(), ' ');
					return s;
				}(n).c_str(),
				Test::Type::VertexThroughput,
				[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t triPerStrip)
				{
					switch(triPerStrip) {
					case 1: cb.bindIndexBuffer(stripPrimitiveRestart3IndexBuffer.get(), 0, vk::IndexType::eUint32); break;
					case 2: cb.bindIndexBuffer(stripPrimitiveRestart4IndexBuffer.get(), 0, vk::IndexType::eUint32); break;
					case 5: cb.bindIndexBuffer(stripPrimitiveRestart7IndexBuffer.get(), 0, vk::IndexType::eUint32); break;
					case 8: cb.bindIndexBuffer(stripPrimitiveRestart10IndexBuffer.get(), 0, vk::IndexType::eUint32); break;
					case 1000: cb.bindIndexBuffer(stripPrimitiveRestart1002IndexBuffer.get(), 0, vk::IndexType::eUint32); break;
					default: assert(0 && "Unhandled triPerStrip parameter."); return;
					};
					beginTest(cb, phongTexturedSingleQuat2PrimitiveRestartPipeline.get(),
					          bufferAndUniformPipelineLayout.get(), timestampIndex,
					          vector<vk::Buffer>{ stripPackedAttribute1.get(), stripPackedAttribute2.get() },
					          vector<vk::DescriptorSet>{ transformationTwoMatricesAndSinglePATDescriptorSet });
					uint32_t numIndicesPerStrip = triPerStrip+3;
					uint32_t numStrips = numTriangles/triPerStrip;
					cb.drawIndexed(numIndicesPerStrip*numStrips, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
					endTest(cb, timestampIndex);
				}
			);
		}

		const char* primitiveRestartPerStripDrawCallPerformanceText =
			"   Primitive restart performance with per-strip vkCmdDrawIndexed() call,\n"
			"      strips of various lengths (1-1000 triangles) each finished by -1,\n"
			"      rendering as triangle strips, textured Phong, constant single\n"
			"      PAT v2 (vec3+vec4) read from the same index in the buffer,\n"
			"      2x uniform matrix (mat4+mat4), packed attributes (2x uvec4),\n"
			"      no fragments produced:                   ";
		for(uint32_t n : array<uint32_t,5>{1,2,5,8,1000}) {  // numbers in this list needs to be divisible by maxTriStripLength (1000 by default) with reminder zero

			tests.emplace_back(
				primitiveRestartPerStripDrawCallPerformanceText,
				n,
				[](uint32_t n) {
					string s = static_cast<stringstream&&>(stringstream() << "         strip length " << n << ": ").str();
					s.append(28-s.size(), ' ');
					return s;
				}(n).c_str(),
				Test::Type::VertexThroughput,
				[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t triPerStrip)
				{
					switch(triPerStrip) {
					case 1: cb.bindIndexBuffer(stripPrimitiveRestart3IndexBuffer.get(), 0, vk::IndexType::eUint32); break;
					case 2: cb.bindIndexBuffer(stripPrimitiveRestart4IndexBuffer.get(), 0, vk::IndexType::eUint32); break;
					case 5: cb.bindIndexBuffer(stripPrimitiveRestart7IndexBuffer.get(), 0, vk::IndexType::eUint32); break;
					case 8: cb.bindIndexBuffer(stripPrimitiveRestart10IndexBuffer.get(), 0, vk::IndexType::eUint32); break;
					case 1000: cb.bindIndexBuffer(stripPrimitiveRestart1002IndexBuffer.get(), 0, vk::IndexType::eUint32); break;
					default: assert(0 && "Unhandled triPerStrip parameter."); return;
					};
					beginTest(cb, phongTexturedSingleQuat2PrimitiveRestartPipeline.get(),
					          bufferAndUniformPipelineLayout.get(), timestampIndex,
					          vector<vk::Buffer>{ stripPackedAttribute1.get(), stripPackedAttribute2.get() },
					          vector<vk::DescriptorSet>{ transformationTwoMatricesAndSinglePATDescriptorSet });
					uint32_t numIndicesPerStrip = triPerStrip+3;
					uint32_t numStrips = numTriangles/triPerStrip;
					for(uint32_t i=0,e=numIndicesPerStrip*numStrips; i<e; i+=numIndicesPerStrip)
						cb.drawIndexed(numIndicesPerStrip, 1, i, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
					endTest(cb, timestampIndex);
				}
			);
		}

		tests.emplace_back(
			"   Primitive restart using 2x -1 followed by indexed triangle,\n"
			"      two vertices reused from previous triangle, single per-scene\n"
			"      vkCmdDrawIndexed() call, textured Phong, constant single\n"
			"      PAT v2 (vec3+vec4) read from the same index in the buffer,\n"
			"      2x uniform matrix (mat4+mat4), packed attributes (2x uvec4),\n"
			"      no fragments produced:                   ",
			Test::Type::VertexThroughput,
			[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
			{
				cb.bindIndexBuffer(primitiveRestartMinusOne2IndexBuffer.get(), 0, vk::IndexType::eUint32);
				beginTest(cb, phongTexturedSingleQuat2PrimitiveRestartPipeline.get(),
				          bufferAndUniformPipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>{ stripPackedAttribute1.get(), stripPackedAttribute2.get() },
				          vector<vk::DescriptorSet>{ transformationTwoMatricesAndSinglePATDescriptorSet });
				cb.drawIndexed(5*numTriangles, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
				endTest(cb, timestampIndex);
			}
		);

		tests.emplace_back(
			"   Primitive restart using 5x -1 followed by indexed triangle,\n"
			"      two vertices reused from previous triangle, per-scene\n"
			"      vkCmdDrawIndexed() call, textured Phong, constant single\n"
			"      PAT v2 (vec3+vec4) read from the same index in the buffer,\n"
			"      2x uniform matrix (mat4+mat4), packed attributes (2x uvec4),\n"
			"      no fragments produced:                   ",
			Test::Type::VertexThroughput,
			[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
			{
				cb.bindIndexBuffer(primitiveRestartMinusOne5IndexBuffer.get(), 0, vk::IndexType::eUint32);
				beginTest(cb, phongTexturedSingleQuat2PrimitiveRestartPipeline.get(),
				          bufferAndUniformPipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>{ stripPackedAttribute1.get(), stripPackedAttribute2.get() },
				          vector<vk::DescriptorSet>{ transformationTwoMatricesAndSinglePATDescriptorSet });
				cb.drawIndexed(8*numTriangles, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
				endTest(cb, timestampIndex);
			}
		);

		tests.emplace_back(
			"   Primitive restart -1 performance, each triangle indices replaced by one -1,\n"
			"      single per-scene vkCmdDrawIndexed() call, per-scene\n"
			"      vkCmdDrawIndexed() call, textured Phong, constant single\n"
			"      PAT v2 (vec3+vec4) read from the same index in the buffer,\n"
			"      2x uniform matrix (mat4+mat4), packed attributes (2x uvec4),\n"
			"      no fragments produced:                   ",
			Test::Type::VertexThroughput,
			[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
			{
				cb.bindIndexBuffer(minusOneIndexBuffer.get(), 0, vk::IndexType::eUint32);
				beginTest(cb, phongTexturedSingleQuat2PrimitiveRestartPipeline.get(),
				          bufferAndUniformPipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>{ stripPackedAttribute1.get(), stripPackedAttribute2.get() },
				          vector<vk::DescriptorSet>{ transformationTwoMatricesAndSinglePATDescriptorSet });
				cb.drawIndexed(numTriangles, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
				endTest(cb, timestampIndex);
			}
		);

		tests.emplace_back(
			"   All 0 index performance, the pipeline uses primitive restart and single\n"
			"      per-scene triangle strip, single per-scene vkCmdDrawIndexed() call,\n"
			"      textured Phong, constant single\n"
			"      PAT v2 (vec3+vec4) read from the same index in the buffer,\n"
			"      2x uniform matrix (mat4+mat4), packed attributes (2x uvec4),\n"
			"      no fragments produced:                   ",
			Test::Type::VertexThroughput,
			[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
			{
				cb.bindIndexBuffer(zeroIndexBuffer.get(), 0, vk::IndexType::eUint32);
				beginTest(cb, phongTexturedSingleQuat2PrimitiveRestartPipeline.get(),
				          bufferAndUniformPipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>{ stripPackedAttribute1.get(), stripPackedAttribute2.get() },
				          vector<vk::DescriptorSet>{ transformationTwoMatricesAndSinglePATDescriptorSet });
				cb.drawIndexed(numTriangles+2, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
				endTest(cb, timestampIndex);
			}
		);

#if 0 // the test is probably not needed
		tests.emplace_back(
			"   All 1 index performance, the pipeline uses primitive restart and single\n"
			"      per-scene triangle strip, single per-scene vkCmdDrawIndexed() call,\n"
			"      textured Phong, constant single\n"
			"      PAT v2 (vec3+vec4) read from the same index in the buffer,\n"
			"      2x uniform matrix (mat4+mat4), packed attributes (2x uvec4),\n"
			"      no fragments produced:                   ",
			Test::Type::VertexThroughput,
			[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
			{
				cb.bindIndexBuffer(plusOneIndexBuffer.get(), 0, vk::IndexType::eUint32);
				beginTest(cb, phongTexturedSingleQuat2PrimitiveRestartPipeline.get(),
				          bufferAndUniformPipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>{ stripPackedAttribute1.get(), stripPackedAttribute2.get() },
				          vector<vk::DescriptorSet>{ transformationTwoMatricesAndSinglePATDescriptorSet });
				cb.drawIndexed(numTriangles+2, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
				endTest(cb, timestampIndex);
			}
		);
#endif

		tests.emplace_back(
			"   All 0 index performance, the pipeline uses single per-scene triangle strip,\n"
			"      single per-scene vkCmdDrawIndexed() call, textured Phong, constant single\n"
			"      PAT v2 (vec3+vec4) read from the same index in the buffer,\n"
			"      2x uniform matrix (mat4+mat4), packed attributes (2x uvec4),\n"
			"      no fragments produced:                   ",
			Test::Type::VertexThroughput,
			[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
			{
				cb.bindIndexBuffer(zeroIndexBuffer.get(), 0, vk::IndexType::eUint32);
				beginTest(cb, phongTexturedSingleQuat2TriStripPipeline.get(),
				          bufferAndUniformPipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>{ stripPackedAttribute1.get(), stripPackedAttribute2.get() },
				          vector<vk::DescriptorSet>{ transformationTwoMatricesAndSinglePATDescriptorSet });
				cb.drawIndexed(numTriangles+2, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
				endTest(cb, timestampIndex);
			}
		);

#if 0 // the test is probably not needed
		tests.emplace_back(
			"   All 1 index performance, the pipeline uses single per-scene triangle strip,\n"
			"      single per-scene vkCmdDrawIndexed() call, textured Phong, constant single\n"
			"      PAT v2 (vec3+vec4) read from the same index in the buffer,\n"
			"      2x uniform matrix (mat4+mat4), packed attributes (2x uvec4),\n"
			"      no fragments produced:                   ",
			Test::Type::VertexThroughput,
			[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
			{
				cb.bindIndexBuffer(plusOneIndexBuffer.get(), 0, vk::IndexType::eUint32);
				beginTest(cb, phongTexturedSingleQuat2TriStripPipeline.get(),
				          bufferAndUniformPipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>{ stripPackedAttribute1.get(), stripPackedAttribute2.get() },
				          vector<vk::DescriptorSet>{ transformationTwoMatricesAndSinglePATDescriptorSet });
				cb.drawIndexed(numTriangles+2, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
				endTest(cb, timestampIndex);
			}
		);
#endif

		tests.emplace_back(
			"   All 0 index performance, the pipeline uses indexed triangle list,\n"
			"      single per-scene vkCmdDrawIndexed() call, textured Phong, constant single\n"
			"      PAT v2 (vec3+vec4) read from the same index in the buffer,\n"
			"      2x uniform matrix (mat4+mat4), packed attributes (2x uvec4),\n"
			"      no fragments produced:                   ",
			Test::Type::VertexThroughput,
			[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
			{
				cb.bindIndexBuffer(zeroIndexBuffer.get(), 0, vk::IndexType::eUint32);
				beginTest(cb, phongTexturedSingleQuat2Pipeline.get(),
				          bufferAndUniformPipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>{ stripPackedAttribute1.get(), stripPackedAttribute2.get() },
				          vector<vk::DescriptorSet>{ transformationTwoMatricesAndSinglePATDescriptorSet });
				cb.drawIndexed(3*numTriangles, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
				endTest(cb, timestampIndex);
			}
		);

#if 0 // the test is probably not needed
		tests.emplace_back(
			"   All 1 index performance, the pipeline uses indexed triangle list,\n"
			"      single per-scene vkCmdDrawIndexed() call, textured Phong, constant single\n"
			"      PAT v2 (vec3+vec4) read from the same index in the buffer,\n"
			"      2x uniform matrix (mat4+mat4), packed attributes (2x uvec4),\n"
			"      no fragments produced:                   ",
			Test::Type::VertexThroughput,
			[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
			{
				cb.bindIndexBuffer(plusOneIndexBuffer.get(), 0, vk::IndexType::eUint32);
				beginTest(cb, phongTexturedSingleQuat2Pipeline.get(),
				          bufferAndUniformPipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>{ stripPackedAttribute1.get(), stripPackedAttribute2.get() },
				          vector<vk::DescriptorSet>{ transformationTwoMatricesAndSinglePATDescriptorSet });
				cb.drawIndexed(3*numTriangles, 1, 0, 0, 0);  // indexCount, instanceCount, firstIndex, vertexOffset, firstInstance
				endTest(cb, timestampIndex);
			}
		);
#endif

		tests.emplace_back(
			"   The same vertex triangle strip performance - all the scene vertices are\n"
			"      the same, each strip is composed of 1000 triangles, per-strip\n"
			"      vkCmdDraw() call, textured Phong, constant single\n"
			"      PAT v2 (vec3+vec4) read from the same index in the buffer,\n"
			"      2x uniform matrix (mat4+mat4), packed attributes (2x uvec4),\n"
			"      no fragments produced:                   ",
			Test::Type::VertexThroughput,
			[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
			{
				beginTest(cb, phongTexturedSingleQuat2TriStripPipeline.get(),
				          bufferAndUniformPipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>{ sameVertexPackedAttribute1.get(), sameVertexPackedAttribute2.get() },
				          vector<vk::DescriptorSet>{ transformationTwoMatricesAndSinglePATDescriptorSet });
				for(uint32_t i=0,e=(numTriangles/maxTriStripLength)*(2+maxTriStripLength); i<e; i+=2+maxTriStripLength)
					cb.draw(2+maxTriStripLength, 1, i, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
				endTest(cb, timestampIndex);
			}
		);

		tests.emplace_back(
			"   The same vertex triangle list performance - all the scene vertices are\n"
			"      the same for the whole scene, single per-scene vkCmdDraw() call,\n"
			"      textured Phong, constant single PAT v2 (vec3+vec4) read from the same\n"
			"      index in the buffer, 2x uniform matrix (mat4+mat4), packed attributes\n"
			"      (2x uvec4), no fragments produced:       ",
			Test::Type::VertexThroughput,
			[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
			{
				beginTest(cb, phongTexturedSingleQuat2Pipeline.get(),
				          bufferAndUniformPipelineLayout.get(), timestampIndex,
				          vector<vk::Buffer>{ sameVertexPackedAttribute1.get(), sameVertexPackedAttribute2.get() },
				          vector<vk::DescriptorSet>{ transformationTwoMatricesAndSinglePATDescriptorSet });
				cb.draw(3*numTriangles, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
				endTest(cb, timestampIndex);
			}
		);
	}


	tests.emplace_back(
		"   Single fullscreen quad, constant color FS:  ",
		Test::Type::FragmentThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			tests[timestampIndex/2].numRenderedItems = 1.;
			beginTest(cb, fillrateContantColorPipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			cb.draw(4, 1, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   10x fullscreen quad, constant color FS:     ",
		Test::Type::FragmentThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			tests[timestampIndex/2].numRenderedItems = numFullscreenQuads;
			beginTest(cb, fillrateContantColorPipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			cb.draw(4, numFullscreenQuads, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Four smooth interpolators (4x vec4),\n"
		"      10x fullscreen quad:                     ",
		Test::Type::FragmentThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			tests[timestampIndex/2].numRenderedItems = numFullscreenQuads;
			beginTest(cb, fillrateFourSmoothInterpolatorsPipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			cb.draw(4, numFullscreenQuads, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Four flat interpolators (4x vec4),\n"
		"      10x fullscreen quad:                     ",
		Test::Type::FragmentThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			tests[timestampIndex/2].numRenderedItems = numFullscreenQuads;
			beginTest(cb, fillrateFourFlatInterpolatorsPipeline.get(), simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			cb.draw(4, numFullscreenQuads, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Four textured phong interpolators (vec3+vec3+vec4+vec2),\n"
		"      10x fullscreen quad:                     ",
		Test::Type::FragmentThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			tests[timestampIndex/2].numRenderedItems = numFullscreenQuads;
			beginTest(cb, fillrateTexturedPhongInterpolatorsPipeline.get(),
			          simplePipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>());
			cb.draw(4, numFullscreenQuads, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Textured Phong, packed uniforms, four smooth interpolators\n"
		"      (vec3+vec3+vec4+vec2), 4x uniform (material (56 byte) +\n"
		"      globalAmbientLight (12 byte) + light (64 byte) + sampler2D),\n"
		"      10x fullscreen quad:                     ",
		Test::Type::FragmentThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			tests[timestampIndex/2].numRenderedItems = numFullscreenQuads;
			beginTest(cb, fillrateTexturedPhongPipeline.get(), phongTexturedPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>{ phongTexturedDescriptorSet });
			cb.draw(4, numFullscreenQuads, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Textured Phong, not packed uniforms, four smooth interpolators\n"
		"      (vec3+vec3+vec4+vec2), 4x uniform (material (72 byte) +\n"
		"      globalAmbientLight (12 byte) + light (80 byte) + sampler2D),\n"
		"      10x fullscreen quad:                     ",
		Test::Type::FragmentThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			tests[timestampIndex/2].numRenderedItems = numFullscreenQuads;
			beginTest(cb, fillrateTexturedPhongNotPackedPipeline.get(),
			          phongTexturedPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>{ phongTexturedNotPackedDescriptorSet });
			cb.draw(4, numFullscreenQuads, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Constant color from uniform, 1x uniform (vec4) in FS,\n"
		"      10x fullscreen quad:                     ",
		Test::Type::FragmentThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			tests[timestampIndex/2].numRenderedItems = numFullscreenQuads;
			beginTest(cb, fillrateUniformColor4fPipeline.get(), oneUniformFSPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>{ one4fUniformFSDescriptorSet });
			cb.draw(4, numFullscreenQuads, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Constant color from uniform, 1x uniform (uint) in FS,\n"
		"      10x fullscreen quad:                     ",
		Test::Type::FragmentThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			tests[timestampIndex/2].numRenderedItems = numFullscreenQuads;
			beginTest(cb, fillrateUniformColor4bPipeline.get(), oneUniformFSPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>{ one4bUniformFSDescriptorSet });
			cb.draw(4, numFullscreenQuads, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Phong, no texture, no specular, 2x smooth interpolator (vec3+vec3),\n"
		"      3x uniform (material (vec4+vec4) + globalAmbientLight (vec3) +\n"
		"      light (48 bytes: position+attenuation+ambient+diffuse)),\n"
		"      10x fullscreen quad:                     ",
		Test::Type::FragmentThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			tests[timestampIndex/2].numRenderedItems = numFullscreenQuads;
			beginTest(cb, phongNoSpecularPipeline.get(), threeUniformFSPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>{ threeUniformFSDescriptorSet });
			cb.draw(4, numFullscreenQuads, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	tests.emplace_back(
		"   Phong, no texture, no specular, 2x smooth interpolator (vec3+vec3),\n"
		"      1x uniform (material+globalAmbientLight+light (vec4+vec4+vec4 +\n"
		"      3x vec4), 10x fullscreen quad:           ",
		Test::Type::FragmentThroughput,
		[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t)
		{
			tests[timestampIndex/2].numRenderedItems = numFullscreenQuads;
			beginTest(cb, phongNoSpecularSingleUniformPipeline.get(),
			          oneUniformFSPipelineLayout.get(), timestampIndex,
			          vector<vk::Buffer>(),
			          vector<vk::DescriptorSet>{ allInOneLightingUniformDescriptorSet });
			cb.draw(4, numFullscreenQuads, 0, 0);  // vertexCount, instanceCount, firstVertex, firstInstance
			endTest(cb, timestampIndex);
		}
	);

	vector<uint32_t> transferSizeList;
	if(minimalTest)
		transferSizeList = { 4, 32 };
	else if(longTest)
		transferSizeList = { 4, 4, 8, 16, 32, 64, 128, 256, 512, 1024,
		                     2048, 4096, 8192, 16384, 32768, 65536,
		                     131072, 262144 };
	else
		transferSizeList = { 4, 32, 256, 2048, 65536 };

	const char* consecutiveTransfersText =
		"   Transfer of consecutive blocks:";
	for(uint32_t n : transferSizeList)
	{
		tests.emplace_back(
			consecutiveTransfersText,
			n,
			[](uint32_t n) {
				string s = static_cast<stringstream&&>(stringstream() << "      " << n << " bytes: ").str();
				return s;
			}(n).c_str(),
			Test::Type::TransferThroughput,
			[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t transferSize)
			{
				// compute numTranfers
				size_t numTransfers = size_t(262144) / transferSize;
				if(minimalTest && numTransfers > 10)
					numTransfers = 10;
				if(sameDMatrixStagingBufferSize < 262144)
					numTransfers = sameDMatrixStagingBufferSize / transferSize;

				// enable TransferThroughput tests only on each fourth measurement
				// because they are usually very time consuming
				bool testEnabled = numTransfers > 0 &&
				                   (tests.front().renderingTimes.size() & 0x3) == 0;

				// set test params
				tests[timestampIndex/2].numTransfers = numTransfers;
				tests[timestampIndex/2].transferSize = transferSize;
				tests[timestampIndex/2].enabled = testEnabled;

				// record command buffer
				beginTestBarrier(cb);
				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eTopOfPipe,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);

				if(testEnabled) {
					for(size_t offset=0, endOffset=transferSize*numTransfers;
						offset<endOffset; offset+=transferSize)
					{
						cb.copyBuffer(
							sameDMatrixStagingBuffer.get(),  // srcBuffer
							sameDMatrixBuffer.get(),         // dstBuffer
							1,                               // regionCount
							&(const vk::BufferCopy&)vk::BufferCopy(  // pRegions
								offset,  // srcOffset
								offset,  // dstOffset
								transferSize  // size
							)
						);
					}
				}

				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eTransfer,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);
			}
		);
	}

	const char* spacedTransfersText =
		"   Transfer of spaced blocks:";
	for(uint32_t n : transferSizeList)
	{
		tests.emplace_back(
			spacedTransfersText,
			n,
			[](uint32_t n) {
				string s = static_cast<stringstream&&>(stringstream() << "      " << n << " bytes: ").str();
				return s;
			}(n).c_str(),
			Test::Type::TransferThroughput,
			[](vk::CommandBuffer cb, uint32_t timestampIndex, uint32_t transferSize)
			{
				// compute numTranfers
				size_t numTransfers = size_t(262144) / transferSize;
				if(minimalTest && numTransfers > 10)
					numTransfers = 10;
				if(sameDMatrixStagingBufferSize < 262144*2)
					numTransfers = sameDMatrixStagingBufferSize / transferSize / 2;

				// enable TransferThroughput tests only on each fourth measurement
				// because they are usually very time consuming
				bool testEnabled = numTransfers > 0 &&
				                   (tests.front().renderingTimes.size() & 0x3) == 0;

				// set test params
				tests[timestampIndex/2].numTransfers = numTransfers;
				tests[timestampIndex/2].transferSize = transferSize;
				tests[timestampIndex/2].enabled = testEnabled;

				// record command buffer
				beginTestBarrier(cb);
				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eTopOfPipe,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);

				if(testEnabled) {
					for(size_t offset=0, endOffset=numTransfers*transferSize*2;
						offset<endOffset; offset+=size_t(transferSize)*2)
					{
						cb.copyBuffer(
							sameDMatrixStagingBuffer.get(),  // srcBuffer
							sameDMatrixBuffer.get(),         // dstBuffer
							1,                               // regionCount
							&(const vk::BufferCopy&)vk::BufferCopy(  // pRegions
								offset,  // srcOffset
								offset,  // dstOffset
								transferSize  // size
							)
						);
					}
				}

				cb.writeTimestamp(
					vk::PipelineStageFlagBits::eTransfer,  // pipelineStage
					timestampPool.get(),  // queryPool
					timestampIndex++      // query
				);
			}
		);
	}

	// assign timestampIndex to each test
	uint32_t timestampIndex = 0;
	for(Test& t : tests) {
		t.timestampIndex = timestampIndex;
		timestampIndex += 2;
	}

	// create shuffled tests list
	// (tests are shuffled before each run;
	// this avoids some problems on Radeons when one test might cause
	// following test to perform poorly probably because some parts of the GPU are
	// switched into powersaving states because of not high enough load)
	shuffledTests.reserve(tests.size());
	for(Test& t : tests)
		shuffledTests.emplace_back(&t);
}


static size_t getBufferSize(uint32_t numTriangles,bool useVec4)
{
	return size_t(numTriangles)*3*(useVec4?4:3)*sizeof(float);
}


static void generateCoordinates(float* vertices,uint32_t numTriangles,unsigned triangleSize,
                                unsigned regionWidth,unsigned regionHeight,bool useVec4,
                                double scaleX=1.,double scaleY=1.,double offsetX=0.,double offsetY=0.)
{
	unsigned stride=triangleSize+2;
	unsigned numTrianglesPerLine=regionWidth/stride*2;
	unsigned numLinesPerScreen=regionHeight/stride;
	size_t idx=0;
	size_t idxEnd=size_t(numTriangles)*(useVec4?4:3)*3;

	// Each iteration generates two triangles.
	// triangleSize is dimension of square that is cut into the two triangles.
	// When triangleSize is set to 2:
	//    Both triangles together produce 4 pixels: the first triangle 3 pixel and
	//    the second triangle 1 pixel. For more detail, see OpenGL rasterization rules.
	// When triangleSize is set to 1:
	//    Both triangles together produce 1 pixel: the first triangle 1 pixel and
	//    the second triangle 0 pixels. For more detail, see OpenGL rasterization rules.
	// When triangleSize is set to 0:
	//    Both triangles together produce 0 pixels: the first triangle 0 pixels and
	//    the second triangle 0 pixels. For more detail, see OpenGL rasterization rules.
	for(float z=0.9f; z>0.01f; z-=0.01f) {
		for(unsigned j=0; j<numLinesPerScreen; j++) {
			for(unsigned i=0; i<numTrianglesPerLine; i+=2) {

				double x=i/2*stride;
				double y=j*stride;

				// triangle 1, vertex 1
				vertices[idx++]=float((x+0.4)*scaleX+offsetX);
				vertices[idx++]=float((y+0.4)*scaleY+offsetY);
				vertices[idx++]=z;
				if(useVec4)
					vertices[idx++]=1.f;

				// triangle 1, vertex 2
				vertices[idx++]=float((x-0.3+triangleSize)*scaleX+offsetX);
				vertices[idx++]=float((y+0.4)*scaleY+offsetY);
				vertices[idx++]=z;
				if(useVec4)
					vertices[idx++]=1.f;

				// triangle 1, vertex 3
				vertices[idx++]=float((x+0.4)*scaleX+offsetX);
				vertices[idx++]=float((y-0.3+triangleSize)*scaleY+offsetY);
				vertices[idx++]=z;
				if(useVec4)
					vertices[idx++]=1.f;

				if(idx==idxEnd)
					return;

				// triangle 2, vertex 1
				vertices[idx++]=float((x+1.1+triangleSize)*scaleX+offsetX);
				vertices[idx++]=float((y+1.7)*scaleY+offsetY);
				vertices[idx++]=z;
				if(useVec4)
					vertices[idx++]=1.f;

				// triangle 2, vertex 2
				vertices[idx++]=float((x+1.7)*scaleX+offsetX);
				vertices[idx++]=float((y+1.1+triangleSize)*scaleY+offsetY);
				vertices[idx++]=z;
				if(useVec4)
					vertices[idx++]=1.f;

				// triangle 2, vertex 3
				vertices[idx++]=float((x+1.1+triangleSize)*scaleX+offsetX);
				vertices[idx++]=float((y+1.1+triangleSize)*scaleY+offsetY);
				vertices[idx++]=z;
				if(useVec4)
					vertices[idx++]=1.f;

				if(idx==idxEnd)
					return;
			}
		}
	}

	// throw if we did not managed to put all the triangles in designed area
	throw std::runtime_error("Triangles do not fit into the rendered area.");
}


static void generateMatrices(float* matrices,uint32_t numMatrices,unsigned triangleSize,
                             unsigned regionWidth,unsigned regionHeight,
                             double scaleX=1.,double scaleY=1.,double offsetX=0.,double offsetY=0.)
{
	unsigned stride=triangleSize+2;
	unsigned numTrianglesPerLine=regionWidth/stride;
	unsigned numLinesPerScreen=regionHeight/stride;
	size_t idx=0;
	size_t idxEnd=size_t(numMatrices)*16;

	for(float z=0.9f; z>0.01f; z-=0.01f) {
		for(unsigned j=0; j<numLinesPerScreen; j++) {
			for(unsigned i=0; i<numTrianglesPerLine; i++) {

				double x=i*stride;
				double y=j*stride;

				float m[]{
					1.f,0.f,0.f,0.f,
					0.f,1.f,0.f,0.f,
					0.f,0.f,1.f,0.f,
					float(x*scaleX+offsetX), float(y*scaleY+offsetY), z-0.9f, 1.f
				};
				memcpy(&matrices[idx],m,sizeof(m));
				idx+=16;

				if(idx==idxEnd)
					return;

			}
		}
	}

	// throw if we did not managed to put all the triangles in designed area
	throw std::runtime_error("Triangles do not fit into the rendered area.");
}


static size_t getBufferSize(uint32_t numStrips,uint32_t numTrianglesInStrip,bool useVec4)
{
	return (2+numTrianglesInStrip)*size_t(numStrips)*(useVec4?4:3)*sizeof(float);
}


static void generateStrips(float* vertices,uint32_t numStrips,uint32_t numTrianglesInStrip,unsigned triangleSize,
                           unsigned regionWidth,unsigned regionHeight,bool useVec4,
                           double scaleX=1.,double scaleY=1.,double offsetX=0.,double offsetY=0.)
{
	unsigned spaceBetweenStrips=1;
	unsigned numStripsPerLine=regionWidth/(numTrianglesInStrip+spaceBetweenStrips);
	unsigned numLinesPerScreen=regionHeight/(triangleSize+1)-1;

	size_t idx=0;
	uint32_t stripCounter=0;

	for(float z=0.9f; z>0.01f; z-=0.01f) {

		for(unsigned k=0; k<numLinesPerScreen; k++) {

			double y=double(k*(triangleSize+1));
			double x=0.;

			for(unsigned j=0; j<numStripsPerLine; j++) {

				// starting triangle, vertex 1
				vertices[idx++]=float((x+0.1)*scaleX+offsetX);
				vertices[idx++]=float((y+0.6)*scaleY+offsetY);
				vertices[idx++]=z;
				if(useVec4)
					vertices[idx++]=1.f;

				// starting triangle, vertex 2
				vertices[idx++]=float((x+0.1)*scaleX+offsetX);
				vertices[idx++]=float((y+0.7+triangleSize)*scaleY+offsetY);
				vertices[idx++]=z;
				if(useVec4)
					vertices[idx++]=1.f;

				for(unsigned i=0; i<numTrianglesInStrip;) {

					x+=1.0;

					// vertex 3
					vertices[idx++]=float((x+0.1)*scaleX+offsetX);
					vertices[idx++]=float((y+0.6)*scaleY+offsetY);
					vertices[idx++]=z;
					if(useVec4)
						vertices[idx++]=1.f;

					i++;
					if(i>=numTrianglesInStrip)
						break;

					// vertex 4
					vertices[idx++]=float((x+0.1)*scaleX+offsetX);
					vertices[idx++]=float((y+0.7+triangleSize)*scaleY+offsetY);
					vertices[idx++]=z;
					if(useVec4)
						vertices[idx++]=1.f;

					i++;

				}

				x+=spaceBetweenStrips;

				stripCounter++;
				if(stripCounter>=numStrips)
					return;
			}
		}
	}

	// throw if we did not managed to put all the triangles in designed area
	throw std::runtime_error("Triangle strips do not fit into the rendered area.");
}


static size_t getIndexBufferSize(uint32_t numStrips,uint32_t numTrianglesInStrip)
{
	return 3*size_t(numTrianglesInStrip)*numStrips*sizeof(uint32_t);
}


static void generateStripIndices(uint32_t* indices,uint32_t numStrips,uint32_t numTrianglesInStrip)
{
	size_t idx=0;

	for(uint32_t j=0; j<numStrips; j++) {
		for(uint32_t i=j*(numTrianglesInStrip+2),e=i+numTrianglesInStrip; i<e; i++) {
			indices[idx++]=i;
			indices[idx++]=i+1;
			indices[idx++]=i+2;
		}
	}
}


static size_t getStripIndexPrimitiveRestartBufferSize(uint32_t numStrips, uint32_t numTrianglesInStrip, uint32_t numMinusOnesBetweenStrips = 1)
{
	return size_t(numStrips) * (numTrianglesInStrip+2+numMinusOnesBetweenStrips) * sizeof(uint32_t);
}


static void generateStripPrimitiveRestartIndices(uint32_t* indices, uint32_t numStrips, uint32_t numTrianglesInStrip, uint32_t numMinusOnesBetweenStrips = 1)
{
	if(maxTriStripLength % numTrianglesInStrip != 0)
		throw runtime_error("generateStripPrimitiveRestartIndices(): numTrianglesInStrip must match "
		                    "maxTriStripLength in a way to be able to generate all the strips of the same "
		                    "length without no excessive small strip at the end.");

	uint32_t numStripsInBigStrip = maxTriStripLength / numTrianglesInStrip;
	uint32_t numBigStrips = numStrips / numStripsInBigStrip;
	uint32_t numIndicesInStrip = numTrianglesInStrip + numMinusOnesBetweenStrips;
	uint32_t numIndicesInBigStrip = numIndicesInStrip * numStripsInBigStrip;
	size_t idx = 0;
	uint32_t i = 0;

	for(uint32_t k=0; k<numBigStrips; k++) {
		for(uint32_t j=0; j<numStripsInBigStrip; j++) {
			for(uint32_t k=0; k<numTrianglesInStrip; k++)
				indices[idx++] = i + k;
			indices[idx++] = i + numTrianglesInStrip;
			indices[idx++] = i + numTrianglesInStrip + 1;
			for(uint32_t k=0; k<numMinusOnesBetweenStrips; k++)
				indices[idx++] = -1;
			i += numTrianglesInStrip;
		}
		i += 2;
	}

	assert(i == numBigStrips*((numStripsInBigStrip*numTrianglesInStrip)+2) && "This should not happen.");
}


static size_t getBufferSizeForSharedVertexTriangles(uint32_t numStrips,uint32_t numTrianglesInStrip,bool useVec4)
{
	return 3*size_t(numTrianglesInStrip)*numStrips*(useVec4?4:3)*sizeof(float);
}


static void generateSharedVertexTriangles(
		float* vertices,uint32_t numStrips,uint32_t numTrianglesInStrip,unsigned triangleSize,
		unsigned regionWidth,unsigned regionHeight,bool useVec4,
		double scaleX=1.,double scaleY=1.,double offsetX=0.,double offsetY=0.)
{
	unsigned spaceBetweenStrips=1;
	unsigned numStripsPerLine=regionWidth/(numTrianglesInStrip+spaceBetweenStrips);
	unsigned numLinesPerScreen=regionHeight/(triangleSize+1)-1;

	size_t idx=0;
	uint32_t stripCounter=0;

	for(float z=0.9f; z>0.01f; z-=0.01f) {

		for(unsigned k=0; k<numLinesPerScreen; k++) {

			double y=double(k*(triangleSize+1));
			double x=0.;

			for(unsigned j=0; j<numStripsPerLine; j++) {

				for(unsigned i=0; i<numTrianglesInStrip;) {

					// triangle 1, vertex 1
					vertices[idx++]=float((x+0.1)*scaleX+offsetX);
					vertices[idx++]=float((y+0.6)*scaleY+offsetY);
					vertices[idx++]=z;
					if(useVec4)
						vertices[idx++]=1.f;

					// triangle 1, vertex 2
					auto idx12=idx;
					vertices[idx++]=float((x+0.1)*scaleX+offsetX);
					vertices[idx++]=float((y+0.7+triangleSize)*scaleY+offsetY);
					vertices[idx++]=z;
					if(useVec4)
						vertices[idx++]=1.f;

					x+=1.0;

					// triangle 1, vertex 3
					auto idx13=idx;
					vertices[idx++]=float((x+0.1)*scaleX+offsetX);
					vertices[idx++]=float((y+0.6)*scaleY+offsetY);
					vertices[idx++]=z;
					if(useVec4)
						vertices[idx++]=1.f;

					i++;
					if(i>=numTrianglesInStrip)
						break;

					// triangle 2, vertex 1
					vertices[idx++]=vertices[idx13++];
					vertices[idx++]=vertices[idx13++];
					vertices[idx++]=z;
					if(useVec4)
						vertices[idx++]=1.f;

					// triangle 2, vertex 2
					vertices[idx++]=vertices[idx12++];
					vertices[idx++]=vertices[idx12++];
					vertices[idx++]=z;
					if(useVec4)
						vertices[idx++]=1.f;

					// triangle 2, vertex 3
					vertices[idx++]=float((x+0.1)*scaleX+offsetX);
					vertices[idx++]=float((y+0.7+triangleSize)*scaleY+offsetY);
					vertices[idx++]=z;
					if(useVec4)
						vertices[idx++]=1.f;

					i++;

				}

				x+=spaceBetweenStrips;

				stripCounter++;
				if(stripCounter>=numStrips)
					return;
			}
		}
	}

	// throw if we did not managed to put all the triangles in designed area
	throw std::runtime_error("Triangle strips do not fit into the rendered area.");
}


// allocate memory for buffers
static tuple<vk::UniqueDeviceMemory,size_t> allocateMemory(vk::Buffer buffer,vk::MemoryPropertyFlags requiredFlags,size_t memorySizeDivisor)
{
	vk::MemoryRequirements memoryRequirements=device->getBufferMemoryRequirements(buffer);
	if(memorySizeDivisor!=1)
		memoryRequirements.size=(memoryRequirements.size/memorySizeDivisor+memoryBlockSize)&memoryBlockMask; // memoryBlockSize example value: 65536 and memoryBlockMask: ~0xffff

	vk::PhysicalDeviceMemoryProperties memoryProperties=physicalDevice.getMemoryProperties();
	for(uint32_t i=0; i<memoryProperties.memoryTypeCount; i++)
		if(memoryRequirements.memoryTypeBits&(1<<i))
			if((memoryProperties.memoryTypes[i].propertyFlags&requiredFlags)==requiredFlags)
				return tuple(
					device->allocateMemoryUnique(
						vk::MemoryAllocateInfo(
							memoryRequirements.size,  // allocationSize
							i                         // memoryTypeIndex
						)
					),
					memoryRequirements.size);
	throw std::runtime_error("No suitable memory type found for the buffer.");
}


// allocate memory for images
static vk::UniqueDeviceMemory allocateMemory(vk::Image image,vk::MemoryPropertyFlags requiredFlags)
{
	vk::MemoryRequirements memoryRequirements=device->getImageMemoryRequirements(image);
	vk::PhysicalDeviceMemoryProperties memoryProperties=physicalDevice.getMemoryProperties();
	for(uint32_t i=0; i<memoryProperties.memoryTypeCount; i++)
		if(memoryRequirements.memoryTypeBits&(1<<i))
			if((memoryProperties.memoryTypes[i].propertyFlags&requiredFlags)==requiredFlags)
				return
					device->allocateMemoryUnique(
						vk::MemoryAllocateInfo(
							memoryRequirements.size,  // allocationSize
							i                         // memoryTypeIndex
						)
					);
	throw std::runtime_error("No suitable memory type found for the image.");
}


/// Returns memory alignment for particular device, memory size and buffer flags.
static size_t getMemoryAlignment(vk::Device device,size_t size,vk::BufferCreateFlags flags)
{
	return
		device.getBufferMemoryRequirements(
			device.createBufferUnique(
				vk::BufferCreateInfo(
					flags,                        // flags
					size,                         // size
					vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eTransferDst,  // usage
					vk::SharingMode::eExclusive,  // sharingMode
					0,                            // queueFamilyIndexCount
					nullptr                       // pQueueFamilyIndices
				)
			).get())
			.alignment;
}


/// Init Vulkan and open the window.
static void init(const string& nameFilter = "", int deviceIndex = -1)
{
	// Vulkan version
	// (vkEnumerateInstanceVersion() is available on Vulkan 1.1+ only. On Vulkan 1.0, it is nullptr.)
	struct VkFunc {
		auto getVkHeaderVersion() const { return VK_HEADER_VERSION; }
		PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion = PFN_vkEnumerateInstanceVersion(vk::Instance().getProcAddr("vkEnumerateInstanceVersion"));
	} vkFunc;
	uint32_t instanceVersion = (vkFunc.vkEnumerateInstanceVersion==nullptr) ? VK_MAKE_VERSION(1,0,0) : vk::enumerateInstanceVersion(vkFunc);

	// physical_device_properties2 support
	bool physicalDeviceProperties2Supported = false;
	auto extensionList = vk::enumerateInstanceExtensionProperties();
	for(vk::ExtensionProperties& e : extensionList) {
		if(strcmp(e.extensionName, "VK_KHR_get_physical_device_properties2") == 0) {
			physicalDeviceProperties2Supported = true;
			break;
		}
	}

	// Vulkan instance
	instance =
		vk::createInstanceUnique(
			vk::InstanceCreateInfo{
				vk::InstanceCreateFlags(),  // flags
				&(const vk::ApplicationInfo&)vk::ApplicationInfo{
					appName.c_str(),  // application name
					appVersion,       // application version
					nullptr,                 // engine name
					VK_MAKE_VERSION(0,0,0),  // engine version
					(instanceVersion < VK_API_VERSION_1_1)  // api version
						? VK_API_VERSION_1_0
						: VK_API_VERSION_1_1,  // version 1.1 is needed for Vulkan Validation Layers, particularly GPU-Assisted validation
				},
				0, nullptr,  // no layers
				1,  // enabled extension count
				array<const char*, 1>{ "VK_KHR_get_physical_device_properties2" }.data(),  // enabled extension names
			});

	// print physical devices
	cout << "Devices in the system:" << endl;
	vector<vk::PhysicalDevice> deviceList = instance->enumeratePhysicalDevices();
	vector<vk::PhysicalDeviceProperties> devicePropertiesList;
	if(deviceList.empty()) {
		cout << "   < no devices found >" << endl;
		throw runtime_error("No Vulkan devices found in the system.");
	}
	for(vk::PhysicalDevice pd : deviceList) {
		devicePropertiesList.emplace_back(pd.getProperties());
		cout << "   " << devicePropertiesList.back().deviceName << endl;
	}

	// find compatible devices
	vector<tuple<vk::PhysicalDevice, uint32_t, uint32_t, vk::PhysicalDeviceProperties,
		vector<vk::ExtensionProperties>>> compatibleDevices;
	for(size_t di=0, c=deviceList.size(); di<c; di++)
	{
		vk::PhysicalDevice pd = deviceList[di];

		// select queues
		uint32_t graphicsQueueFamily = UINT32_MAX;
		uint32_t sparseQueueFamily = UINT32_MAX;
		vector<vk::QueueFamilyProperties> queueFamilyList = pd.getQueueFamilyProperties();
		extensionList = pd.enumerateDeviceExtensionProperties();
		uint32_t i = 0;
		for(auto it=queueFamilyList.begin(); it!=queueFamilyList.end(); it++,i++) {
			if(it->queueFlags & vk::QueueFlagBits::eGraphics) {
				if(it->queueFlags & vk::QueueFlagBits::eSparseBinding) {
					compatibleDevices.emplace_back(pd, i, i, devicePropertiesList[di], move(extensionList));
					goto nextDevice;
				}
				else {
					if(graphicsQueueFamily == UINT32_MAX)
						graphicsQueueFamily = i;
				}
			}
			else {
				if(sparseQueueFamily == UINT32_MAX && it->queueFlags & vk::QueueFlagBits::eSparseBinding)
					sparseQueueFamily = i;
			}
		}
		if(sparseMode == SPARSE_NONE) {
			if(graphicsQueueFamily != UINT32_MAX)
				compatibleDevices.emplace_back(pd, graphicsQueueFamily, sparseQueueFamily,
				                               devicePropertiesList[di], move(extensionList));
		}
		else
			if(graphicsQueueFamily != UINT32_MAX && sparseQueueFamily != UINT32_MAX)
				compatibleDevices.emplace_back(pd, graphicsQueueFamily, sparseQueueFamily,
				                               devicePropertiesList[di], move(extensionList));
		nextDevice:;
	}
	if(compatibleDevices.empty())
		throw runtime_error("No compatible devices.");

	// filter physical devices
	if(!nameFilter.empty())
	{
		decltype(compatibleDevices) filteredDevices;
		for(auto& d : compatibleDevices)
			if(string_view(std::get<3>(d).deviceName).find(nameFilter) != string::npos)
				filteredDevices.push_back(d);

		compatibleDevices.swap(filteredDevices);
	}
	if(compatibleDevices.empty())
		throw runtime_error("No device name matching the filter \"" + nameFilter + "\".");

	if(deviceIndex >= 0)
	{
		// choose by index
		if(deviceIndex < decltype(deviceIndex)(compatibleDevices.size())) {
			auto& d = compatibleDevices[deviceIndex];
			physicalDevice = std::get<0>(d);
			graphicsQueueFamily = std::get<1>(d);
			sparseQueueFamily = std::get<2>(d);
			physicalDeviceProperties = std::get<3>(d);
			physicalDeviceExtensions = std::move(get<4>(d));
		}
		else
			throw runtime_error("Invalid device index. "
				"Index " + to_string(deviceIndex) + " is not in the valid range 0.." +
				to_string(compatibleDevices.size()) + ".");
	}
	else
	{
		// choose the best device
		auto bestDevice = compatibleDevices.begin();
		if(bestDevice == compatibleDevices.end())
			throw runtime_error("No device selected.");
		constexpr const array deviceTypeScore = {
			10, // vk::PhysicalDeviceType::eOther         - lowest score
			40, // vk::PhysicalDeviceType::eIntegratedGpu - high score
			50, // vk::PhysicalDeviceType::eDiscreteGpu   - highest score
			30, // vk::PhysicalDeviceType::eVirtualGpu    - normal score
			20, // vk::PhysicalDeviceType::eCpu           - low score
			10, // unknown vk::PhysicalDeviceType
		};
		int bestScore = deviceTypeScore[clamp(int(std::get<3>(*bestDevice).deviceType), 0, int(deviceTypeScore.size())-1)];
		if(std::get<1>(*bestDevice) == std::get<2>(*bestDevice))
			bestScore++;
		for(auto it=compatibleDevices.begin()+1; it!=compatibleDevices.end(); it++) {
			int score = deviceTypeScore[clamp(int(std::get<3>(*it).deviceType), 0, int(deviceTypeScore.size())-1)];
			if(std::get<1>(*it) == std::get<2>(*it))
				score++;
			if(score > bestScore) {
				bestDevice = it;
				bestScore = score;
			}
		}

		physicalDevice = std::get<0>(*bestDevice);
		graphicsQueueFamily = std::get<1>(*bestDevice);
		sparseQueueFamily = std::get<2>(*bestDevice);
		physicalDeviceProperties = std::get<3>(*bestDevice);
		physicalDeviceExtensions = move(std::get<4>(*bestDevice));
	}
	cout << "\nSelected device:\n"
	        "   " << physicalDeviceProperties.deviceName << "\n" << endl;

	// get supported features
	vk::PhysicalDeviceFeatures physicalFeatures = physicalDevice.getFeatures();
	enabledFeatures.setMultiDrawIndirect(physicalFeatures.multiDrawIndirect);
	enabledFeatures.setGeometryShader(physicalFeatures.geometryShader);
	enabledFeatures.setShaderFloat64(physicalFeatures.shaderFloat64);

	// DeviceID and VendorID
	cout << "VendorID:  0x" << hex << physicalDeviceProperties.vendorID;
	switch(physicalDeviceProperties.vendorID) {
	case 0x1002: cout << " (AMD/ATI)" << endl; break;
	case 0x10DE: cout << " (Nvidia)" << endl; break;
	case 0x8086: cout << " (Intel)" << endl; break;
	case 0x10005: cout << " (Mesa)"; break;
	default: cout << endl;
	}
	cout << "DeviceID:  0x" << physicalDeviceProperties.deviceID << dec << endl;

	// driver info
	cout << "Vulkan version:  " << VK_VERSION_MAJOR(physicalDeviceProperties.apiVersion) << "."
	     << VK_VERSION_MINOR(physicalDeviceProperties.apiVersion) << "."
	     << VK_VERSION_PATCH(physicalDeviceProperties.apiVersion) << endl;
	cout << "Driver version:  ";
	if(physicalDeviceProperties.vendorID == 0x10DE)
		// Nvidia uses 10|8|8|6
		cout << ((physicalDeviceProperties.driverVersion >> 22) & 0x3ff) << '.'
		     << ((physicalDeviceProperties.driverVersion >> 14) & 0x0ff) << '.'
		     << ((physicalDeviceProperties.driverVersion >>  6) & 0x0ff) << '.'
		     << ((physicalDeviceProperties.driverVersion >>  0) & 0x03f);
#ifdef _WIN32
	else if(physicalDeviceProperties.vendorID == 0x8086)
		// Intel uses 18|14 on Win32
		cout << (physicalDeviceProperties.driverVersion >> 14) << '.'
		     << (physicalDeviceProperties.driverVersion & 0x3fff);
#endif
	else
		// try standard Vulkan versioning scheme, e.g. 10|10|12 
		cout <<  (physicalDeviceProperties.driverVersion >> 22) << '.'
		     << ((physicalDeviceProperties.driverVersion >> 12) & 0x3ff) << '.'
		     << ((physicalDeviceProperties.driverVersion >>  0) & 0xfff);
	cout << " (" << physicalDeviceProperties.driverVersion
	     << ", 0x" << hex << physicalDeviceProperties.driverVersion << ")" << dec << endl;
	for(const vk::ExtensionProperties& e : physicalDeviceExtensions)
		if(strcmp(e.extensionName, "VK_KHR_driver_properties") == 0) {
#if VK_HEADER_VERSION>=186
			struct InstanceFuncs : vk::DispatchLoaderBase {
#else
			struct InstanceFuncs {
#endif
				PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR = PFN_vkGetPhysicalDeviceProperties2KHR(instance->getProcAddr("vkGetPhysicalDeviceProperties2KHR"));
			} vkFuncs;
			auto properties2 = physicalDevice.getProperties2KHR<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceDriverPropertiesKHR>(vkFuncs);
			vk::PhysicalDeviceDriverPropertiesKHR& driverProperties = properties2.get<vk::PhysicalDeviceDriverPropertiesKHR>();
			cout << "   Driver name:  " << driverProperties.driverName << endl;
			cout << "   Driver info:  " << driverProperties.driverInfo << endl;
			cout << "   DriverID:     " << to_string(driverProperties.driverID) << endl;
			cout << "   Driver conformance version:  " << unsigned(driverProperties.conformanceVersion.major)
			     << "." << unsigned(driverProperties.conformanceVersion.minor) 
			     << "." << unsigned(driverProperties.conformanceVersion.subminor)
			     << "." << unsigned(driverProperties.conformanceVersion.patch) << endl;
			goto afterDriverProperties;
		}
	cout << "   Driver name:  n/a" << endl;
	cout << "   Driver info:  n/a" << endl;
	cout << "   DriverID:     n/a" << endl;
	cout << "   Driver conformance version:  n/a" << endl;
	afterDriverProperties:

	// GPU memory and sparse props
	size_t gpuMemory=0;
	{
		vk::PhysicalDeviceMemoryProperties memoryProperties=physicalDevice.getMemoryProperties();
		for(uint32_t i=0; i<memoryProperties.memoryHeapCount; i++)
			if(memoryProperties.memoryHeaps[i].flags&vk::MemoryHeapFlagBits::eDeviceLocal)
				gpuMemory+=memoryProperties.memoryHeaps[i].size;
		cout<<"GPU memory:  "<<(((gpuMemory+512)/1024+512)/1024+512)/1024<<"GiB  (";
		bool first=true;
		for(uint32_t i=0; i<memoryProperties.memoryHeapCount; i++)
			if(memoryProperties.memoryHeaps[i].flags&vk::MemoryHeapFlagBits::eDeviceLocal) {
				if(first)  first=false;
				else  cout<<" + ";
				cout<<((memoryProperties.memoryHeaps[i].size+512)/1024+512)/1024<<"MiB";
			}
		cout<<")"<<endl;
	}
	cout << "Max memory allocations:  " << physicalDeviceProperties.limits.maxMemoryAllocationCount << endl;

	switch(sparseMode) {
	case SPARSE_NONE:
		break;
	case SPARSE_BINDING:
		if(!physicalFeatures.sparseBinding)
			throw runtime_error("Sparse binding is not supported.");
		enabledFeatures.setSparseBinding(true);
		bufferCreateFlags = vk::BufferCreateFlagBits::eSparseBinding;
		bufferSizeMultiplier = 1;
		break;
	case SPARSE_RESIDENCY:
		if(!physicalFeatures.sparseBinding || !physicalFeatures.sparseResidencyBuffer)
			throw runtime_error("Sparse residency is not supported.");
		enabledFeatures.setSparseBinding(true);
		enabledFeatures.setSparseResidencyBuffer(true);
		bufferCreateFlags = vk::BufferCreateFlagBits::eSparseBinding | vk::BufferCreateFlagBits::eSparseResidency;
		bufferSizeMultiplier = 10;
		break;
	case SPARSE_RESIDENCY_ALIASED:
		if(!physicalFeatures.sparseBinding || !physicalFeatures.sparseResidencyBuffer)
			throw runtime_error("Sparse residency is not supported.");
		enabledFeatures.setSparseBinding(true);
		enabledFeatures.setSparseResidencyBuffer(true);
		if(!physicalFeatures.sparseResidencyAliased)
			throw runtime_error("Sparse aliased is not supported.");
		enabledFeatures.setSparseResidencyAliased(true);
		bufferCreateFlags = vk::BufferCreateFlagBits::eSparseBinding|vk::BufferCreateFlagBits::eSparseResidency | vk::BufferCreateFlagBits::eSparseAliased;
		bufferSizeMultiplier = 10;
		break;
	}

	// create device
	device=
		physicalDevice.createDeviceUnique(
			vk::DeviceCreateInfo{
				vk::DeviceCreateFlags(),  // flags
				(sparseMode==SPARSE_NONE) ? uint32_t(1) : uint32_t(2), // queueCreateInfoCount
				array<const vk::DeviceQueueCreateInfo,2>{  // pQueueCreateInfos
					vk::DeviceQueueCreateInfo{
						vk::DeviceQueueCreateFlags(),
						graphicsQueueFamily,
						1,
						&(const float&)1.f,
					},
					vk::DeviceQueueCreateInfo{
						vk::DeviceQueueCreateFlags(),
						sparseQueueFamily,
						1,
						&(const float&)1.f,
					},
				}.data(),
				0,nullptr,  // no layers
				0,        // number of enabled extensions
				nullptr,  // enabled extension names
				&enabledFeatures,  // enabled features
			}
		);

	// get queues
	graphicsQueue=device->getQueue(graphicsQueueFamily,0);
	if(sparseQueueFamily!=UINT32_MAX)
		sparseQueue=device->getQueue(sparseQueueFamily,0);

	// print memory alignment
	cout<<"Standard (non-sparse) buffer alignment:  "
	    <<getMemoryAlignment(device.get(),1,vk::BufferCreateFlags())<<endl;

	// set sparse block variables
	if(sparseMode==SPARSE_NONE) {
		memoryBlockSize=1;
		memoryBlockMask=~0;
	} else {
		memoryBlockSize=getMemoryAlignment(device.get(),1,bufferCreateFlags);
		memoryBlockMask=~(memoryBlockSize-1);
		sparseBlockSize=memoryBlockSize;
	}

	// number of triangles
	// (reduce the number of triangles based on amount of graphics memory as we might easily run out of it)
	if(minimalTest)
		numTriangles = numTrianglesMinimal;
	else {
		if(gpuMemory >= 5.6*1024*1024*1024) // >=5.6GiB
			numTriangles = numTrianglesStandard;
		else if(gpuMemory >= 2.8*1024*1024*1024) // >=2.8GiB
			numTriangles = numTrianglesStandard/2;
		else if(gpuMemory >= 1.4*1024*1024*1024) // >=1.4GiB
			numTriangles = numTrianglesStandard/5;
		else if(gpuMemory >= 700*1024*1024) // >=700MiB
			numTriangles = numTrianglesStandard/10;
		else if(gpuMemory >= 400*1024*1024) // >=400MiB
			numTriangles = numTrianglesStandard/20;
		else
			numTriangles = numTrianglesStandard/50;

		// put further limits on integrated and cpu-emulated devices
		if(physicalDeviceProperties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu)
			numTriangles = min(numTriangles, numTrianglesIntegratedGpu);
		else if(physicalDeviceProperties.deviceType == vk::PhysicalDeviceType::eCpu)
			numTriangles = min(numTriangles, numTrianglesCpu);
	}
	cout << "Number of triangles for tests:  " << numTriangles << endl;

	// print sparse mode for tests
	switch(sparseMode) {
	case SPARSE_NONE:              cout<<"Sparse mode for tests:  None"<<endl; break;
	case SPARSE_BINDING:           cout<<"Sparse mode for tests:  Binding"<<endl; break;
	case SPARSE_RESIDENCY:         cout<<"Sparse mode for tests:  Residency"<<endl; break;
	case SPARSE_RESIDENCY_ALIASED: cout<<"Sparse mode for tests:  ResidencyAliased"<<endl; break;
	default: assert(0 && "This should never happen.");
	}

	// timestamp properties
	timestampValidBits=
		physicalDevice.getQueueFamilyProperties()[graphicsQueueFamily].timestampValidBits;
	timestampPeriod_ns=
		physicalDeviceProperties.limits.timestampPeriod;
	cout<<"Timestamp number of bits:  "<<timestampValidBits<<endl;
	cout<<"Timestamp period:  "<<timestampPeriod_ns<<"ns"<<endl;
	if(timestampValidBits==0)
		throw runtime_error("Timestamps are not supported.");

	// Instance version
	cout << "Vulkan Instance version:  " << VK_VERSION_MAJOR(instanceVersion) << "."
	     << VK_VERSION_MINOR(instanceVersion) << "." << VK_VERSION_PATCH(instanceVersion) << endl;

	cout << "Operating system:  ";
#ifdef _WIN32
	{
		// read OS version from registry
		HKEY hKey;
		LONG r = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey);
		if(r == ERROR_SUCCESS) {
			vector<char> buf;
			DWORD size;
			r = RegQueryValueExA(hKey, "ProductName", 0, NULL, NULL, &size);
			if(r == ERROR_SUCCESS) {
				size += 1; // space for terminating character
				buf.resize(size+1);
				r = RegQueryValueExA(hKey, "ProductName", 0, NULL, reinterpret_cast<LPBYTE>(buf.data()), &size);
				if(r == ERROR_SUCCESS) {
					buf[size] = 0;
					cout << buf.data();
				} else
					cout << "< registry error, unknown Windows >";
			}
			else
				cout << "< registry error, unknown Windows >";
			if(RegCloseKey(hKey) != ERROR_SUCCESS)
				cout << "  (registry error)";
		}
		else
			cout << "< registry error, unknown Windows >";

		// get "Nt" version numbers
		cout << "\n                   Version number: ";
		HMODULE h = GetModuleHandleW(L"ntdll.dll"); // since WinXP
		if(h == nullptr)
			goto osInfoFailed;
		using RtlGetNtVersionNumbersPtr = void (*)(DWORD* majorVersion, DWORD* minorVersion, DWORD* buildNumber);
		RtlGetNtVersionNumbersPtr RtlGetNtVersionNumbers = reinterpret_cast<RtlGetNtVersionNumbersPtr>(GetProcAddress(h, "RtlGetNtVersionNumbers")); // since WinXP
		if(RtlGetNtVersionNumbers == nullptr)
			goto osInfoFailed;
		DWORD majorVersion = 0;
		DWORD minorVersion = 0;
		DWORD buildNumber = 0;
		RtlGetNtVersionNumbers(&majorVersion, &minorVersion, &buildNumber);
		cout << majorVersion << "." << minorVersion
		     << ", build number: " << (buildNumber&0xffff) << endl;
		goto osInfoSucceed;
	}
osInfoFailed:
	cout << "< unknown >" << endl;
osInfoSucceed:;
	cout << "Processor:  ";
	{
		union {
			char brandString[4*12+1] = {0};
			int cpuInfo[12];
		};
		__cpuid(cpuInfo, 0x80000000);
		if(cpuInfo[0] >= 0x80000004) {
			__cpuid(&cpuInfo[0], 0x80000002);
			__cpuid(&cpuInfo[4], 0x80000003);
			__cpuid(&cpuInfo[8], 0x80000004);
			cout << brandString << endl;
		}
		else
			cout << "< unknown >" << endl;
	}
#else
	{
		ifstream f("/etc/lsb-release");
		if(!f)  goto osInfoFailed;
		string s{istreambuf_iterator<char>(f), istreambuf_iterator<char>()};
		if(!f)  goto osInfoFailed;
		regex expr("(?:^|\\n|\\r)[\\t ]*DISTRIB_DESCRIPTION[\\t ]*=[\\t ]*(?:\"([^\\r\\n:]*)\"|([^\\r\\n:]*))", std::regex_constants::icase);
		sregex_iterator it(s.begin(), s.end(), expr);
		if(it == sregex_iterator{})  goto osInfoFailed;
		if((*it)[1] != "")
			cout << (*it)[1] << endl;
		else
			cout << (*it)[2] << endl;
	}
	goto osInfoSucceed;
osInfoFailed:
	cout << "< unknown, non-Windows >" << endl;
osInfoSucceed:;
	cout << "Processor:  ";
	{
		ifstream f("/proc/cpuinfo");
		if(!f)  goto cpuInfoFailed;
		string s{istreambuf_iterator<char>(f), istreambuf_iterator<char>()};
		if(!f)  goto cpuInfoFailed;
		regex expr("(?:^|\\n|\\r)[\\t ]*model\\ name[\\t ]*\\:[\\t ]*([^\\r\\n:]*)", std::regex_constants::icase);
		sregex_iterator it(s.begin(), s.end(), expr);
		if(it == sregex_iterator{})  goto cpuInfoFailed;
		cout << (*it)[1] << endl;
	}
	goto cpuInfoSucceed;
cpuInfoFailed:
	cout << "< unknown >" << endl;
cpuInfoSucceed:;
#endif
	cout << endl;

	// choose surface format
	depthFormat=
		[](){
			for(vk::Format f:array<vk::Format,3>{vk::Format::eD32Sfloat,vk::Format::eD32SfloatS8Uint,vk::Format::eD24UnormS8Uint}) {
				vk::FormatProperties p=physicalDevice.getFormatProperties(f);
				if(p.optimalTilingFeatures&vk::FormatFeatureFlagBits::eDepthStencilAttachment)
					return f;
			}
			throw std::runtime_error("No suitable depth buffer format.");
		}();

	// render pass
	renderPass=
		device->createRenderPassUnique(
			vk::RenderPassCreateInfo(
				vk::RenderPassCreateFlags(),  // flags
				2,                            // attachmentCount
				array<const vk::AttachmentDescription,2>{  // pAttachments
					vk::AttachmentDescription{  // color attachment
						vk::AttachmentDescriptionFlags(),  // flags
						colorFormat,                       // format
						vk::SampleCountFlagBits::e1,       // samples
						vk::AttachmentLoadOp::eClear,      // loadOp
						vk::AttachmentStoreOp::eStore,     // storeOp
						vk::AttachmentLoadOp::eDontCare,   // stencilLoadOp
						vk::AttachmentStoreOp::eDontCare,  // stencilStoreOp
						vk::ImageLayout::eUndefined,       // initialLayout
						vk::ImageLayout::eColorAttachmentOptimal  // finalLayout
					},
					vk::AttachmentDescription{  // depth attachment
						vk::AttachmentDescriptionFlags(),  // flags
						depthFormat,                       // format
						vk::SampleCountFlagBits::e1,       // samples
						vk::AttachmentLoadOp::eClear,      // loadOp
						vk::AttachmentStoreOp::eDontCare,  // storeOp
						vk::AttachmentLoadOp::eDontCare,   // stencilLoadOp
						vk::AttachmentStoreOp::eDontCare,  // stencilStoreOp
						vk::ImageLayout::eUndefined,       // initialLayout
						vk::ImageLayout::eDepthStencilAttachmentOptimal  // finalLayout
					}
				}.data(),
				1,  // subpassCount
				&(const vk::SubpassDescription&)vk::SubpassDescription(  // pSubpasses
					vk::SubpassDescriptionFlags(),     // flags
					vk::PipelineBindPoint::eGraphics,  // pipelineBindPoint
					0,        // inputAttachmentCount
					nullptr,  // pInputAttachments
					1,        // colorAttachmentCount
					&(const vk::AttachmentReference&)vk::AttachmentReference(  // pColorAttachments
						0,  // attachment
						vk::ImageLayout::eColorAttachmentOptimal  // layout
					),
					nullptr,  // pResolveAttachments
					&(const vk::AttachmentReference&)vk::AttachmentReference(  // pDepthStencilAttachment
						1,  // attachment
						vk::ImageLayout::eDepthStencilAttachmentOptimal  // layout
					),
					0,        // preserveAttachmentCount
					nullptr   // pPreserveAttachments
				),
				1,      // dependencyCount
				array{  // pDependencies
					vk::SubpassDependency(
						VK_SUBPASS_EXTERNAL,   // srcSubpass
						0,                     // dstSubpass
#if 1 // set to 0 to enable full pipeline barrier (for debugging purposes)
						vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput |  // srcStageMask
						                       vk::PipelineStageFlagBits::eEarlyFragmentTests),
						vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput |  // dstStageMask
						                       vk::PipelineStageFlagBits::eEarlyFragmentTests),
						vk::AccessFlags(vk::AccessFlagBits::eColorAttachmentWrite |  // srcAccessMask
						                vk::AccessFlagBits::eDepthStencilAttachmentWrite),
						vk::AccessFlags(vk::AccessFlagBits::eColorAttachmentWrite |  // dstAccessMask
						                vk::AccessFlagBits::eDepthStencilAttachmentWrite),
						vk::DependencyFlags()  // dependencyFlags
#else
						vk::PipelineStageFlags(vk::PipelineStageFlagBits::eAllCommands),  // srcStageMask
						vk::PipelineStageFlags(vk::PipelineStageFlagBits::eAllCommands),  // dstStageMask
						vk::AccessFlags(vk::AccessFlagBits::eMemoryRead |  // srcAccessMask
						                vk::AccessFlagBits::eMemoryWrite),
						vk::AccessFlags(vk::AccessFlagBits::eMemoryRead |  // dstAccessMask
						                vk::AccessFlagBits::eMemoryWrite),
#endif
					),
				}.data()
			)
		);

	// create shader modules
	attributelessConstantOutputVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),  // flags
				sizeof(attributelessConstantOutputVS_spirv),  // codeSize
				attributelessConstantOutputVS_spirv           // pCode
			)
		);
	attributelessInputIndicesVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),  // flags
				sizeof(attributelessInputIndicesVS_spirv),  // codeSize
				attributelessInputIndicesVS_spirv           // pCode
			)
		);
	coordinateAttributeVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),        // flags
				sizeof(coordinateAttributeVS_spirv),  // codeSize
				coordinateAttributeVS_spirv           // pCode
			)
		);
	coordinate4BufferVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),      // flags
				sizeof(coordinate4BufferVS_spirv),  // codeSize
				coordinate4BufferVS_spirv           // pCode
			)
		);
	coordinate3BufferVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),      // flags
				sizeof(coordinate3BufferVS_spirv),  // codeSize
				coordinate3BufferVS_spirv           // pCode
			)
		);
	singleUniformMatrixVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),        // flags
				sizeof(singleUniformMatrixVS_spirv),  // codeSize
				singleUniformMatrixVS_spirv           // pCode
			)
		);
	matrixAttributeVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),    // flags
				sizeof(matrixAttributeVS_spirv),  // codeSize
				matrixAttributeVS_spirv           // pCode
			)
		);
	matrixBufferVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),  // flags
				sizeof(matrixBufferVS_spirv),   // codeSize
				matrixBufferVS_spirv            // pCode
			)
		);
	twoAttributesVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),  // flags
				sizeof(twoAttributesVS_spirv),  // codeSize
				twoAttributesVS_spirv           // pCode
			)
		);
	twoBuffersVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),  // flags
				sizeof(twoBuffersVS_spirv),     // codeSize
				twoBuffersVS_spirv              // pCode
			)
		);
	twoBuffer3VS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),  // flags
				sizeof(twoBuffer3VS_spirv),     // codeSize
				twoBuffer3VS_spirv              // pCode
			)
		);
	twoInterleavedBuffersVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),          // flags
				sizeof(twoInterleavedBuffersVS_spirv),  // codeSize
				twoInterleavedBuffersVS_spirv           // pCode
			)
		);
	twoPackedAttributesVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),        // flags
				sizeof(twoPackedAttributesVS_spirv),  // codeSize
				twoPackedAttributesVS_spirv           // pCode
			)
		);
	twoPackedBuffersVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),     // flags
				sizeof(twoPackedBuffersVS_spirv),  // codeSize
				twoPackedBuffersVS_spirv           // pCode
			)
		);
	twoPackedBuffersUsingStructVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                // flags
				sizeof(twoPackedBuffersUsingStructVS_spirv),  // codeSize
				twoPackedBuffersUsingStructVS_spirv           // pCode
			)
		);
	twoPackedBuffersUsingStructSlowVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                    // flags
				sizeof(twoPackedBuffersUsingStructSlowVS_spirv),  // codeSize
				twoPackedBuffersUsingStructSlowVS_spirv           // pCode
			)
		);
	singlePackedBufferVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),       // flags
				sizeof(singlePackedBufferVS_spirv),  // codeSize
				singlePackedBufferVS_spirv           // pCode
			)
		);
	twoPackedAttributesAndSingleMatrixVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                       // flags
				sizeof(twoPackedAttributesAndSingleMatrixVS_spirv),  // codeSize
				twoPackedAttributesAndSingleMatrixVS_spirv           // pCode
			)
		);
	twoPackedAttributesAndMatrixVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                 // flags
				sizeof(twoPackedAttributesAndMatrixVS_spirv),  // codeSize
				twoPackedAttributesAndMatrixVS_spirv           // pCode
			)
		);
	twoPackedBuffersAndMatrixVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),              // flags
				sizeof(twoPackedBuffersAndMatrixVS_spirv),  // codeSize
				twoPackedBuffersAndMatrixVS_spirv           // pCode
			)
		);
	fourAttributesVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),   // flags
				sizeof(fourAttributesVS_spirv),  // codeSize
				fourAttributesVS_spirv           // pCode
			)
		);
	fourBuffersVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),  // flags
				sizeof(fourBuffersVS_spirv),    // codeSize
				fourBuffersVS_spirv             // pCode
			)
		);
	fourBuffer3VS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),  // flags
				sizeof(fourBuffer3VS_spirv),    // codeSize
				fourBuffer3VS_spirv             // pCode
			)
		);
	fourInterleavedBuffersVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),           // flags
				sizeof(fourInterleavedBuffersVS_spirv),  // codeSize
				fourInterleavedBuffersVS_spirv           // pCode
			)
		);
	fourAttributesAndMatrixVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),            // flags
				sizeof(fourAttributesAndMatrixVS_spirv),  // codeSize
				fourAttributesAndMatrixVS_spirv           // pCode
			)
		);
	geometryShaderConstantOutputVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                 // flags
				sizeof(geometryShaderConstantOutputVS_spirv),  // codeSize
				geometryShaderConstantOutputVS_spirv           // pCode
			)
		);
	geometryShaderConstantOutputGS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                 // flags
				sizeof(geometryShaderConstantOutputGS_spirv),  // codeSize
				geometryShaderConstantOutputGS_spirv           // pCode
			)
		);
	geometryShaderConstantOutputTwoTrianglesGS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                             // flags
				sizeof(geometryShaderConstantOutputTwoTrianglesGS_spirv),  // codeSize
				geometryShaderConstantOutputTwoTrianglesGS_spirv           // pCode
			)
		);
	geometryShaderNoOutputGS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                 // flags
				sizeof(geometryShaderNoOutputGS_spirv),  // codeSize
				geometryShaderNoOutputGS_spirv           // pCode
			)
		);
	geometryShaderVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),   // flags
				sizeof(geometryShaderVS_spirv),  // codeSize
				geometryShaderVS_spirv           // pCode
			)
		);
	geometryShaderGS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),   // flags
				sizeof(geometryShaderGS_spirv),  // codeSize
				geometryShaderGS_spirv           // pCode
			)
		);
	transformationThreeMatricesVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                // flags
				sizeof(transformationThreeMatricesVS_spirv),  // codeSize
				transformationThreeMatricesVS_spirv           // pCode
			)
		);
	transformationFiveMatricesVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),               // flags
				sizeof(transformationFiveMatricesVS_spirv),  // codeSize
				transformationFiveMatricesVS_spirv           // pCode
			)
		);
	transformationFiveMatricesPushConstantsVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),               // flags
				sizeof(transformationFiveMatricesPushConstantsVS_spirv),  // codeSize
				transformationFiveMatricesPushConstantsVS_spirv  // pCode
			)
		);
	transformationFiveMatricesSpecializationConstantsVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),               // flags
				sizeof(transformationFiveMatricesSpecializationConstantsVS_spirv),  // codeSize
				transformationFiveMatricesSpecializationConstantsVS_spirv  // pCode
			)
		);
	transformationFiveMatricesConstantsVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),               // flags
				sizeof(transformationFiveMatricesConstantsVS_spirv),  // codeSize
				transformationFiveMatricesConstantsVS_spirv  // pCode
			)
		);
	transformationFiveMatricesUsingGS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                    // flags
				sizeof(transformationFiveMatricesUsingGS_spirv),  // codeSize
				transformationFiveMatricesUsingGS_spirv           // pCode
			)
		);
	transformationFiveMatricesUsingGSAndAttributesVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                                   // flags
				sizeof(transformationFiveMatricesUsingGSAndAttributesVS_spirv),  // codeSize
				transformationFiveMatricesUsingGSAndAttributesVS_spirv           // pCode
			)
		);
	transformationFiveMatricesUsingGSAndAttributesGS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                                   // flags
				sizeof(transformationFiveMatricesUsingGSAndAttributesGS_spirv),  // codeSize
				transformationFiveMatricesUsingGSAndAttributesGS_spirv           // pCode
			)
		);
	phongTexturedFourAttributesFiveMatricesVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                            // flags
				sizeof(phongTexturedFourAttributesFiveMatricesVS_spirv),  // codeSize
				phongTexturedFourAttributesFiveMatricesVS_spirv           // pCode
			)
		);
	phongTexturedFourAttributesVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                // flags
				sizeof(phongTexturedFourAttributesVS_spirv),  // codeSize
				phongTexturedFourAttributesVS_spirv           // pCode
			)
		);
	phongTexturedVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),  // flags
				sizeof(phongTexturedVS_spirv),  // codeSize
				phongTexturedVS_spirv           // pCode
			)
		);
	phongTexturedRowMajorVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),          // flags
				sizeof(phongTexturedRowMajorVS_spirv),  // codeSize
				phongTexturedRowMajorVS_spirv           // pCode
			)
		);
	phongTexturedMat4x3VS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),        // flags
				sizeof(phongTexturedMat4x3VS_spirv),  // codeSize
				phongTexturedMat4x3VS_spirv           // pCode
			)
		);
	phongTexturedMat4x3RowMajorVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                // flags
				sizeof(phongTexturedMat4x3RowMajorVS_spirv),  // codeSize
				phongTexturedMat4x3RowMajorVS_spirv           // pCode
			)
		);
	phongTexturedQuat1VS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),       // flags
				sizeof(phongTexturedQuat1VS_spirv),  // codeSize
				phongTexturedQuat1VS_spirv           // pCode
			)
		);
	phongTexturedQuat2VS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),       // flags
				sizeof(phongTexturedQuat2VS_spirv),  // codeSize
				phongTexturedQuat2VS_spirv           // pCode
			)
		);
	phongTexturedQuat3VS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),       // flags
				sizeof(phongTexturedQuat3VS_spirv),  // codeSize
				phongTexturedQuat3VS_spirv           // pCode
			)
		);
	phongTexturedQuat2PrimitiveRestartVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                       // flags
				sizeof(phongTexturedQuat2PrimitiveRestartVS_spirv),  // codeSize
				phongTexturedQuat2PrimitiveRestartVS_spirv           // pCode
			)
		);
	phongTexturedSingleQuat2VS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),             // flags
				sizeof(phongTexturedSingleQuat2VS_spirv),  // codeSize
				phongTexturedSingleQuat2VS_spirv           // pCode
			)
		);
	phongTexturedDMatricesOnlyInputVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                    // flags
				sizeof(phongTexturedDMatricesOnlyInputVS_spirv),  // codeSize
				phongTexturedDMatricesOnlyInputVS_spirv           // pCode
			)
		);
	phongTexturedDMatricesVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),           // flags
				sizeof(phongTexturedDMatricesVS_spirv),  // codeSize
				phongTexturedDMatricesVS_spirv           // pCode
			)
		);
	phongTexturedDMatricesDVerticesVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                    // flags
				sizeof(phongTexturedDMatricesDVerticesVS_spirv),  // codeSize
				phongTexturedDMatricesDVerticesVS_spirv           // pCode
			)
		);
	phongTexturedInGSDMatricesDVerticesVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                        // flags
				sizeof(phongTexturedInGSDMatricesDVerticesVS_spirv),  // codeSize
				phongTexturedInGSDMatricesDVerticesVS_spirv           // pCode
			)
		);
	phongTexturedInGSDMatricesDVerticesGS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                        // flags
				sizeof(phongTexturedInGSDMatricesDVerticesGS_spirv),  // codeSize
				phongTexturedInGSDMatricesDVerticesGS_spirv           // pCode
			)
		);
	constantColorFS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),  // flags
				sizeof(constantColorFS_spirv),  // codeSize
				constantColorFS_spirv           // pCode
			)
		);
	phongTexturedDummyFS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),       // flags
				sizeof(phongTexturedDummyFS_spirv),  // codeSize
				phongTexturedDummyFS_spirv           // pCode
			)
		);
	phongTexturedFS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),  // flags
				sizeof(phongTexturedFS_spirv),  // codeSize
				phongTexturedFS_spirv           // pCode
			)
		);
	phongTexturedNotPackedFS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),           // flags
				sizeof(phongTexturedNotPackedFS_spirv),  // codeSize
				phongTexturedNotPackedFS_spirv           // pCode
			)
		);
	fullscreenQuadVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),   // flags
				sizeof(fullscreenQuadVS_spirv),  // codeSize
				fullscreenQuadVS_spirv           // pCode
			)
		);
	fullscreenQuadFourInterpolatorsVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                    // flags
				sizeof(fullscreenQuadFourInterpolatorsVS_spirv),  // codeSize
				fullscreenQuadFourInterpolatorsVS_spirv           // pCode
			)
		);
	fullscreenQuadFourSmoothInterpolatorsFS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                          // flags
				sizeof(fullscreenQuadFourSmoothInterpolatorsFS_spirv),  // codeSize
				fullscreenQuadFourSmoothInterpolatorsFS_spirv           // pCode
			)
		);
	fullscreenQuadFourFlatInterpolatorsFS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                        // flags
				sizeof(fullscreenQuadFourFlatInterpolatorsFS_spirv),  // codeSize
				fullscreenQuadFourFlatInterpolatorsFS_spirv           // pCode
			)
		);
	fullscreenQuadTexturedPhongInterpolatorsVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                             // flags
				sizeof(fullscreenQuadTexturedPhongInterpolatorsVS_spirv),  // codeSize
				fullscreenQuadTexturedPhongInterpolatorsVS_spirv           // pCode
			)
		);
	fullscreenQuadTexturedPhongInterpolatorsFS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                             // flags
				sizeof(fullscreenQuadTexturedPhongInterpolatorsFS_spirv),  // codeSize
				fullscreenQuadTexturedPhongInterpolatorsFS_spirv           // pCode
			)
		);
	uniformColor4fFS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),   // flags
				sizeof(uniformColor4fFS_spirv),  // codeSize
				uniformColor4fFS_spirv           // pCode
			)
		);
	uniformColor4bFS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),   // flags
				sizeof(uniformColor4bFS_spirv),  // codeSize
				uniformColor4bFS_spirv           // pCode
			)
		);
	fullscreenQuadTwoVec3InterpolatorsVS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                       // flags
				sizeof(fullscreenQuadTwoVec3InterpolatorsVS_spirv),  // codeSize
				fullscreenQuadTwoVec3InterpolatorsVS_spirv           // pCode
			)
		);
	phongNoSpecularFS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),    // flags
				sizeof(phongNoSpecularFS_spirv),  // codeSize
				phongNoSpecularFS_spirv           // pCode
			)
		);
	phongNoSpecularSingleUniformFS=
		device->createShaderModuleUnique(
			vk::ShaderModuleCreateInfo(
				vk::ShaderModuleCreateFlags(),                 // flags
				sizeof(phongNoSpecularSingleUniformFS_spirv),  // codeSize
				phongNoSpecularSingleUniformFS_spirv           // pCode
			)
		);

	// pipeline cache
	pipelineCache=
		device->createPipelineCacheUnique(
			vk::PipelineCacheCreateInfo(
				vk::PipelineCacheCreateFlags(),  // flags
				0,       // initialDataSize
				nullptr  // pInitialData
			)
		);

	// descriptor set layouts
	oneUniformVSDescriptorSetLayout=
		device->createDescriptorSetLayoutUnique(
			vk::DescriptorSetLayoutCreateInfo(
				vk::DescriptorSetLayoutCreateFlags(),  // flags
				1,  // bindingCount
				array<vk::DescriptorSetLayoutBinding,1>{  // pBindings
					vk::DescriptorSetLayoutBinding(
						0,  // binding
						vk::DescriptorType::eUniformBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eVertex,  // stageFlags
						nullptr  // pImmutableSamplers
					)
				}.data()
			)
		);
	oneUniformFSDescriptorSetLayout=
		device->createDescriptorSetLayoutUnique(
			vk::DescriptorSetLayoutCreateInfo(
				vk::DescriptorSetLayoutCreateFlags(),  // flags
				1,  // bindingCount
				array<vk::DescriptorSetLayoutBinding,1>{  // pBindings
					vk::DescriptorSetLayoutBinding(
						0,  // binding
						vk::DescriptorType::eUniformBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eFragment,  // stageFlags
						nullptr  // pImmutableSamplers
					)
				}.data()
			)
		);
	oneBufferDescriptorSetLayout=
		device->createDescriptorSetLayoutUnique(
			vk::DescriptorSetLayoutCreateInfo(
				vk::DescriptorSetLayoutCreateFlags(),  // flags
				1,  // bindingCount
				array<vk::DescriptorSetLayoutBinding,1>{  // pBindings
					vk::DescriptorSetLayoutBinding(
						0,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eVertex,  // stageFlags
						nullptr  // pImmutableSamplers
					)
				}.data()
			)
		);
	twoBuffersDescriptorSetLayout=
		device->createDescriptorSetLayoutUnique(
			vk::DescriptorSetLayoutCreateInfo(
				vk::DescriptorSetLayoutCreateFlags(),  // flags
				2,  // bindingCount
				array<vk::DescriptorSetLayoutBinding,2>{  // pBindings
					vk::DescriptorSetLayoutBinding(
						0,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eVertex,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						1,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eVertex,  // stageFlags
						nullptr  // pImmutableSamplers
					)
				}.data()
			)
		);
	threeBuffersDescriptorSetLayout=
		device->createDescriptorSetLayoutUnique(
			vk::DescriptorSetLayoutCreateInfo(
				vk::DescriptorSetLayoutCreateFlags(),  // flags
				3,  // bindingCount
				array<vk::DescriptorSetLayoutBinding,3>{  // pBindings
					vk::DescriptorSetLayoutBinding(
						0,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eVertex,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						1,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eVertex,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						2,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eVertex,  // stageFlags
						nullptr  // pImmutableSamplers
					)
				}.data()
			)
		);
	fourBuffersDescriptorSetLayout=
		device->createDescriptorSetLayoutUnique(
			vk::DescriptorSetLayoutCreateInfo(
				vk::DescriptorSetLayoutCreateFlags(),  // flags
				4,  // bindingCount
				array<vk::DescriptorSetLayoutBinding,4>{  // pBindings
					vk::DescriptorSetLayoutBinding(
						0,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eVertex,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						1,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eVertex,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						2,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eVertex,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						3,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eVertex,  // stageFlags
						nullptr  // pImmutableSamplers
					),
				}.data()
			)
		);
	threeBuffersInGSDescriptorSetLayout=
		device->createDescriptorSetLayoutUnique(
			vk::DescriptorSetLayoutCreateInfo(
				vk::DescriptorSetLayoutCreateFlags(),  // flags
				3,  // bindingCount
				array<vk::DescriptorSetLayoutBinding,3>{  // pBindings
					vk::DescriptorSetLayoutBinding(
						0,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eGeometry,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						1,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eGeometry,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						2,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eGeometry,  // stageFlags
						nullptr  // pImmutableSamplers
					)
				}.data()
			)
		);
	threeUniformFSDescriptorSetLayout=
		device->createDescriptorSetLayoutUnique(
			vk::DescriptorSetLayoutCreateInfo(
				vk::DescriptorSetLayoutCreateFlags(),  // flags
				3,  // bindingCount
				array<vk::DescriptorSetLayoutBinding,3>{  // pBindings
					vk::DescriptorSetLayoutBinding(
						0,  // binding
						vk::DescriptorType::eUniformBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eFragment,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						1,  // binding
						vk::DescriptorType::eUniformBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eFragment,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						2,  // binding
						vk::DescriptorType::eUniformBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eFragment,  // stageFlags
						nullptr  // pImmutableSamplers
					)
				}.data()
			)
		);
	bufferAndUniformDescriptorSetLayout=
		device->createDescriptorSetLayoutUnique(
			vk::DescriptorSetLayoutCreateInfo(
				vk::DescriptorSetLayoutCreateFlags(),  // flags
				2,  // bindingCount
				array<vk::DescriptorSetLayoutBinding,2>{  // pBindings
					vk::DescriptorSetLayoutBinding(
						0,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eVertex,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						1,  // binding
						vk::DescriptorType::eUniformBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eVertex,  // stageFlags
						nullptr  // pImmutableSamplers
					)
				}.data()
			)
		);
	bufferAndUniformInGSDescriptorSetLayout=
		device->createDescriptorSetLayoutUnique(
			vk::DescriptorSetLayoutCreateInfo(
				vk::DescriptorSetLayoutCreateFlags(),  // flags
				2,  // bindingCount
				array<vk::DescriptorSetLayoutBinding,2>{  // pBindings
					vk::DescriptorSetLayoutBinding(
						0,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eGeometry,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						1,  // binding
						vk::DescriptorType::eUniformBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eGeometry,  // stageFlags
						nullptr  // pImmutableSamplers
					)
				}.data()
			)
		);
	twoBuffersAndUniformDescriptorSetLayout=
		device->createDescriptorSetLayoutUnique(
			vk::DescriptorSetLayoutCreateInfo(
				vk::DescriptorSetLayoutCreateFlags(),  // flags
				3,  // bindingCount
				array<vk::DescriptorSetLayoutBinding,3>{  // pBindings
					vk::DescriptorSetLayoutBinding(
						0,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eVertex,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						1,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eVertex,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						2,  // binding
						vk::DescriptorType::eUniformBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eVertex,  // stageFlags
						nullptr  // pImmutableSamplers
					)
				}.data()
			)
		);
	twoBuffersAndUniformInGSDescriptorSetLayout=
		device->createDescriptorSetLayoutUnique(
			vk::DescriptorSetLayoutCreateInfo(
				vk::DescriptorSetLayoutCreateFlags(),  // flags
				3,  // bindingCount
				array<vk::DescriptorSetLayoutBinding,3>{  // pBindings
					vk::DescriptorSetLayoutBinding(
						0,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eGeometry,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						1,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eGeometry,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						2,  // binding
						vk::DescriptorType::eUniformBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eGeometry,  // stageFlags
						nullptr  // pImmutableSamplers
					)
				}.data()
			)
		);
	fourBuffersAndUniformInGSDescriptorSetLayout=
		device->createDescriptorSetLayoutUnique(
			vk::DescriptorSetLayoutCreateInfo(
				vk::DescriptorSetLayoutCreateFlags(),  // flags
				5,  // bindingCount
				array<vk::DescriptorSetLayoutBinding,5>{  // pBindings
					vk::DescriptorSetLayoutBinding(
						0,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eGeometry,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						1,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eGeometry,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						2,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eGeometry,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						3,  // binding
						vk::DescriptorType::eStorageBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eGeometry,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						4,  // binding
						vk::DescriptorType::eUniformBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eGeometry,  // stageFlags
						nullptr  // pImmutableSamplers
					)
				}.data()
			)
		);
	phongTexturedDescriptorSetLayout=
		device->createDescriptorSetLayoutUnique(
			vk::DescriptorSetLayoutCreateInfo(
				vk::DescriptorSetLayoutCreateFlags(),  // flags
				4,  // bindingCount
				array<vk::DescriptorSetLayoutBinding,4>{  // pBindings
					vk::DescriptorSetLayoutBinding(
						0,  // binding
						vk::DescriptorType::eUniformBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eFragment,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						1,  // binding
						vk::DescriptorType::eUniformBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eFragment,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						2,  // binding
						vk::DescriptorType::eUniformBuffer,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eFragment,  // stageFlags
						nullptr  // pImmutableSamplers
					),
					vk::DescriptorSetLayoutBinding(
						3,  // binding
						vk::DescriptorType::eCombinedImageSampler,  // descriptorType
						1,  // descriptorCount
						vk::ShaderStageFlagBits::eFragment,  // stageFlags
						nullptr  // pImmutableSamplers
					),
				}.data()
			)
		);

	// pipeline layouts
	simplePipelineLayout=
		device->createPipelineLayoutUnique(
			vk::PipelineLayoutCreateInfo{
				vk::PipelineLayoutCreateFlags(),  // flags
				0,       // setLayoutCount
				nullptr, // pSetLayouts
				0,       // pushConstantRangeCount
				nullptr  // pPushConstantRanges
			}
		);
	oneUniformVSPipelineLayout=
		device->createPipelineLayoutUnique(
			vk::PipelineLayoutCreateInfo{
				vk::PipelineLayoutCreateFlags(),  // flags
				1,       // setLayoutCount
				&oneUniformVSDescriptorSetLayout.get(),  // pSetLayouts
				0,       // pushConstantRangeCount
				nullptr  // pPushConstantRanges
			}
		);
	oneUniformFSPipelineLayout=
		device->createPipelineLayoutUnique(
			vk::PipelineLayoutCreateInfo{
				vk::PipelineLayoutCreateFlags(),  // flags
				1,       // setLayoutCount
				&oneUniformFSDescriptorSetLayout.get(),  // pSetLayouts
				0,       // pushConstantRangeCount
				nullptr  // pPushConstantRanges
			}
		);
	oneBufferPipelineLayout=
		device->createPipelineLayoutUnique(
			vk::PipelineLayoutCreateInfo{
				vk::PipelineLayoutCreateFlags(),  // flags
				1,       // setLayoutCount
				&oneBufferDescriptorSetLayout.get(),  // pSetLayouts
				0,       // pushConstantRangeCount
				nullptr  // pPushConstantRanges
			}
		);
	twoBuffersPipelineLayout=
		device->createPipelineLayoutUnique(
			vk::PipelineLayoutCreateInfo{
				vk::PipelineLayoutCreateFlags(),  // flags
				1,       // setLayoutCount
				&twoBuffersDescriptorSetLayout.get(),  // pSetLayouts
				0,       // pushConstantRangeCount
				nullptr  // pPushConstantRanges
			}
		);
	threeBuffersPipelineLayout=
		device->createPipelineLayoutUnique(
			vk::PipelineLayoutCreateInfo{
				vk::PipelineLayoutCreateFlags(),  // flags
				1,       // setLayoutCount
				&threeBuffersDescriptorSetLayout.get(),  // pSetLayouts
				0,       // pushConstantRangeCount
				nullptr  // pPushConstantRanges
			}
		);
	fourBuffersPipelineLayout=
		device->createPipelineLayoutUnique(
			vk::PipelineLayoutCreateInfo{
				vk::PipelineLayoutCreateFlags(),  // flags
				1,       // setLayoutCount
				&fourBuffersDescriptorSetLayout.get(),  // pSetLayouts
				0,       // pushConstantRangeCount
				nullptr  // pPushConstantRanges
			}
		);
	threeBuffersInGSPipelineLayout=
		device->createPipelineLayoutUnique(
			vk::PipelineLayoutCreateInfo{
				vk::PipelineLayoutCreateFlags(),  // flags
				1,       // setLayoutCount
				&threeBuffersInGSDescriptorSetLayout.get(),  // pSetLayouts
				0,       // pushConstantRangeCount
				nullptr  // pPushConstantRanges
			}
		);
	threeUniformFSPipelineLayout=
		device->createPipelineLayoutUnique(
			vk::PipelineLayoutCreateInfo{
				vk::PipelineLayoutCreateFlags(),  // flags
				1,       // setLayoutCount
				&threeUniformFSDescriptorSetLayout.get(),  // pSetLayouts
				0,       // pushConstantRangeCount
				nullptr  // pPushConstantRanges
			}
		);
	bufferAndUniformPipelineLayout=
		device->createPipelineLayoutUnique(
			vk::PipelineLayoutCreateInfo{
				vk::PipelineLayoutCreateFlags(),  // flags
				1,       // setLayoutCount
				&bufferAndUniformDescriptorSetLayout.get(),  // pSetLayouts
				0,       // pushConstantRangeCount
				nullptr  // pPushConstantRanges
			}
		);
	bufferAndUniformInGSPipelineLayout=
		device->createPipelineLayoutUnique(
			vk::PipelineLayoutCreateInfo{
				vk::PipelineLayoutCreateFlags(),  // flags
				1,       // setLayoutCount
				&bufferAndUniformInGSDescriptorSetLayout.get(),  // pSetLayouts
				0,       // pushConstantRangeCount
				nullptr  // pPushConstantRanges
			}
		);
	twoBuffersAndUniformPipelineLayout=
		device->createPipelineLayoutUnique(
			vk::PipelineLayoutCreateInfo{
				vk::PipelineLayoutCreateFlags(),  // flags
				1,       // setLayoutCount
				&twoBuffersAndUniformDescriptorSetLayout.get(),  // pSetLayouts
				0,       // pushConstantRangeCount
				nullptr  // pPushConstantRanges
			}
		);
	twoBuffersAndPushConstantsPipelineLayout=
		device->createPipelineLayoutUnique(
			vk::PipelineLayoutCreateInfo{
				vk::PipelineLayoutCreateFlags(),  // flags
				1,       // setLayoutCount
				&twoBuffersDescriptorSetLayout.get(),  // pSetLayouts
				1,       // pushConstantRangeCount
				array{  // pPushConstantRanges
					vk::PushConstantRange{
						vk::ShaderStageFlagBits::eVertex,  // stageFlags
						0,  // offset
						128,  // size
					},
				}.data()
			}
		);
	twoBuffersAndUniformInGSPipelineLayout=
		device->createPipelineLayoutUnique(
			vk::PipelineLayoutCreateInfo{
				vk::PipelineLayoutCreateFlags(),  // flags
				1,       // setLayoutCount
				&twoBuffersAndUniformInGSDescriptorSetLayout.get(),  // pSetLayouts
				0,       // pushConstantRangeCount
				nullptr  // pPushConstantRanges
			}
		);
	fourBuffersAndUniformInGSPipelineLayout=
		device->createPipelineLayoutUnique(
			vk::PipelineLayoutCreateInfo{
				vk::PipelineLayoutCreateFlags(),  // flags
				1,       // setLayoutCount
				&fourBuffersAndUniformInGSDescriptorSetLayout.get(),  // pSetLayouts
				0,       // pushConstantRangeCount
				nullptr  // pPushConstantRanges
			}
		);
	phongTexturedPipelineLayout=
		device->createPipelineLayoutUnique(
			vk::PipelineLayoutCreateInfo{
				vk::PipelineLayoutCreateFlags(),  // flags
				1,       // setLayoutCount
				&phongTexturedDescriptorSetLayout.get(),  // pSetLayouts
				0,       // pushConstantRangeCount
				nullptr  // pPushConstantRanges
			}
		);

	// textures
	singleTexelImage=
		device->createImageUnique(
			vk::ImageCreateInfo(
				vk::ImageCreateFlags(),  // flags
				vk::ImageType::e2D,      // imageType
				vk::Format::eR8G8B8A8Unorm,  // format
				vk::Extent3D(1,1,1),     // extent
				1,                       // mipLevels
				1,                       // arrayLayers
				vk::SampleCountFlagBits::e1,  // samples
				vk::ImageTiling::eOptimal,    // tiling
				vk::ImageUsageFlagBits::eSampled|vk::ImageUsageFlagBits::eTransferDst,  // usage
				vk::SharingMode::eExclusive,  // sharingMode
				0,                            // queueFamilyIndexCount
				nullptr,                      // pQueueFamilyIndices
				vk::ImageLayout::eUndefined   // initialLayout
			)
		);
	singleTexelImageMemory=allocateMemory(singleTexelImage.get(),vk::MemoryPropertyFlagBits::eDeviceLocal);
	device->bindImageMemory(
		singleTexelImage.get(),  // image
		singleTexelImageMemory.get(),  // memory
		0  // memoryOffset
	);
	singleTexelImageView=
		device->createImageViewUnique(
			vk::ImageViewCreateInfo(
				vk::ImageViewCreateFlags(),  // flags
				singleTexelImage.get(),      // image
				vk::ImageViewType::e2D,      // viewType
				vk::Format::eR8G8B8A8Unorm,  // format
				vk::ComponentMapping(),      // components
				vk::ImageSubresourceRange(   // subresourceRange
					vk::ImageAspectFlagBits::eColor,  // aspectMask
					0,  // baseMipLevel
					1,  // levelCount
					0,  // baseArrayLayer
					1   // layerCount
				)
			)
		);
	trilinearSampler=
		device->createSamplerUnique(
			vk::SamplerCreateInfo(
				vk::SamplerCreateFlags(),  // flags
				vk::Filter::eLinear,   // magFilter
				vk::Filter::eLinear,   // minFilter
				vk::SamplerMipmapMode::eLinear,  // mipmapMode
				vk::SamplerAddressMode::eClampToEdge,  // addressModeU
				vk::SamplerAddressMode::eClampToEdge,  // addressModeV
				vk::SamplerAddressMode::eClampToEdge,  // addressModeW
				0.f,  // mipLodBias
				VK_FALSE,  // anisotropyEnable
				0.f,  // maxAnisotropy
				VK_FALSE,  // compareEnable
				vk::CompareOp::eNever,  // compareOp
				0.f,  // minLod
				0.f,  // maxLod
				vk::BorderColor::eFloatTransparentBlack,  // borderColor
				VK_FALSE  // unnormalizedCoordinates
			)
		);

	// command pool
	commandPool=
		device->createCommandPoolUnique(
			vk::CommandPoolCreateInfo(
				vk::CommandPoolCreateFlagBits::eTransient,  // flags
				graphicsQueueFamily  // queueFamilyIndex
			)
		);

	// command buffer
	commandBuffer=std::move(
		device->allocateCommandBuffersUnique(
			vk::CommandBufferAllocateInfo(
				commandPool.get(),                 // commandPool
				vk::CommandBufferLevel::ePrimary,  // level
				1   // commandBufferCount
			)
		)[0]);

	// fence
	fence = device->createFenceUnique(
		vk::FenceCreateInfo{
			vk::FenceCreateFlags()
		}
	);
}


/// Resize framebuffer and all dependent objects. The function is usually used on each framebuffer resize and on application start.
static void resizeFramebuffer(vk::Extent2D newExtent)
{
	framebufferExtent=newExtent;

	// stop device and clear resources
	device->waitIdle();
	framebuffer.reset();
	colorImage.reset();
	depthImage.reset();
	colorImageMemory.reset();
	depthImageMemory.reset();
	colorImageView.reset();
	depthImageView.reset();
	attributelessConstantOutputPipeline.reset();
	attributelessConstantOutputTriStripPipeline.reset();
	attributelessConstantOutputPrimitiveRestartPipeline.reset();
	attributelessInputIndicesPipeline.reset();
	attributelessInputIndicesTriStripPipeline.reset();
	attributelessInputIndicesPrimitiveRestartPipeline.reset();
	coordinateAttributePipeline.reset();
	coordinate4BufferPipeline.reset();
	coordinate3BufferPipeline.reset();
	singleMatrixUniformPipeline.reset();
	matrixAttributePipeline.reset();
	matrixBufferPipeline.reset();
	twoAttributesPipeline.reset();
	twoBuffersPipeline.reset();
	twoBuffer3Pipeline.reset();
	twoInterleavedAttributesPipeline.reset();
	twoInterleavedBuffersPipeline.reset();
	two4F32Two4U8AttributesPipeline.reset();
	twoPackedAttributesPipeline.reset();
	twoPackedBuffersPipeline.reset();
	twoPackedBuffersUsingStructPipeline.reset();
	twoPackedBuffersUsingStructSlowPipeline.reset();
	twoPackedAttributesAndSingleMatrixPipeline.reset();
	twoPackedAttributesAndMatrixPipeline.reset();
	twoPackedBuffersAndMatrixPipeline.reset();
	singlePackedBufferPipeline.reset();
	fourAttributesPipeline.reset();
	fourBuffersPipeline.reset();
	fourBuffer3Pipeline.reset();
	fourInterleavedAttributesPipeline.reset();
	fourInterleavedBuffersPipeline.reset();
	fourAttributesAndMatrixPipeline.reset();
	geometryShaderConstantOutputPipeline.reset();
	geometryShaderConstantOutputTwoTrianglesPipeline.reset();
	geometryShaderNoOutputPipeline.reset();
	geometryShaderPipeline.reset();
	transformationThreeMatricesPipeline.reset();
	transformationFiveMatricesPipeline.reset();
	transformationFiveMatricesPushConstantsPipeline.reset();
	transformationFiveMatricesSpecializationConstantsPipeline.reset();
	transformationFiveMatricesConstantsPipeline.reset();
	transformationFiveMatricesUsingGSPipeline.reset();
	transformationFiveMatricesUsingGSAndAttributesPipeline.reset();
	phongTexturedFourAttributesFiveMatricesPipeline.reset();
	phongTexturedFourAttributesPipeline.reset();
	phongTexturedPipeline.reset();
	phongTexturedRowMajorPipeline.reset();
	phongTexturedMat4x3Pipeline.reset();
	phongTexturedMat4x3RowMajorPipeline.reset();
	phongTexturedQuat1Pipeline.reset();
	phongTexturedQuat2Pipeline.reset();
	phongTexturedQuat3Pipeline.reset();
	phongTexturedQuat2PrimitiveRestartPipeline.reset();
	phongTexturedDMatricesOnlyInputPipeline.reset();
	phongTexturedDMatricesPipeline.reset();
	phongTexturedDMatricesDVerticesPipeline.reset();
	phongTexturedInGSDMatricesDVerticesPipeline.reset();
	phongTexturedSingleQuat2Pipeline.reset();
	phongTexturedSingleQuat2TriStripPipeline.reset();
	phongTexturedSingleQuat2PrimitiveRestartPipeline.reset();
	fillrateContantColorPipeline.reset();
	fillrateFourSmoothInterpolatorsPipeline.reset();
	fillrateFourFlatInterpolatorsPipeline.reset();
	fillrateTexturedPhongInterpolatorsPipeline.reset();
	fillrateTexturedPhongPipeline.reset();
	fillrateTexturedPhongNotPackedPipeline.reset();
	fillrateUniformColor4fPipeline.reset();
	fillrateUniformColor4bPipeline.reset();
	phongNoSpecularPipeline.reset();
	phongNoSpecularSingleUniformPipeline.reset();
	coordinate4Attribute.reset();
	coordinate4AttributeMemory.reset();
	coordinate4Buffer.reset();
	coordinate4BufferMemory.reset();
	coordinate3Attribute.reset();
	coordinate3AttributeMemory.reset();
	coordinate3Buffer.reset();
	coordinate3BufferMemory.reset();
	normalAttribute.reset();
	normalAttributeMemory.reset();
	colorAttribute.reset();
	colorAttributeMemory.reset();
	texCoordAttribute.reset();
	texCoordAttributeMemory.reset();
	for(auto& a : vec4Attributes)         a.reset();
	for(auto& m : vec4AttributeMemory)    m.reset();
	for(auto& a : vec4u8Attributes)       a.reset();
	for(auto& m : vec4u8AttributeMemory)  m.reset();
	for(auto& b : vec4Buffers)            b.reset();
	for(auto& m : vec4BufferMemory)       m.reset();
	for(auto& b : vec3Buffers)            b.reset();
	for(auto& m : vec3BufferMemory)       m.reset();
	packedAttribute1.reset();
	packedAttribute1Memory.reset();
	packedAttribute2.reset();
	packedAttribute2Memory.reset();
	twoInterleavedAttributes.reset();
	twoInterleavedAttributesMemory.reset();
	twoInterleavedBuffers.reset();
	twoInterleavedBuffersMemory.reset();
	fourInterleavedAttributes.reset();
	fourInterleavedAttributesMemory.reset();
	fourInterleavedBuffers.reset();
	fourInterleavedBuffersMemory.reset();
	packedBuffer1.reset();
	packedBuffer1Memory.reset();
	packedBuffer2.reset();
	packedBuffer2Memory.reset();
	packedDAttribute1.reset();
	packedDAttribute1Memory.reset();
	packedDAttribute2.reset();
	packedDAttribute2Memory.reset();
	packedDAttribute3.reset();
	packedDAttribute3Memory.reset();
	indexBuffer.reset();
	indexBufferMemory.reset();
	primitiveRestartIndexBuffer.reset();
	primitiveRestartIndexBufferMemory.reset();
	stripIndexBuffer.reset();
	stripIndexBufferMemory.reset();
	stripPrimitiveRestart3IndexBuffer.reset();
	stripPrimitiveRestart3IndexBufferMemory.reset();
	stripPrimitiveRestart4IndexBuffer.reset();
	stripPrimitiveRestart4IndexBufferMemory.reset();
	stripPrimitiveRestart7IndexBuffer.reset();
	stripPrimitiveRestart7IndexBufferMemory.reset();
	stripPrimitiveRestart10IndexBuffer.reset();
	stripPrimitiveRestart10IndexBufferMemory.reset();
	stripPrimitiveRestart1002IndexBuffer.reset();
	stripPrimitiveRestart1002IndexBufferMemory.reset();
	primitiveRestartMinusOne2IndexBuffer.reset();
	primitiveRestartMinusOne2IndexBufferMemory.reset();
	primitiveRestartMinusOne5IndexBuffer.reset();
	primitiveRestartMinusOne5IndexBufferMemory.reset();
	minusOneIndexBuffer.reset();
	minusOneIndexBufferMemory.reset();
	zeroIndexBuffer.reset();
	zeroIndexBufferMemory.reset();
	plusOneIndexBuffer.reset();
	plusOneIndexBufferMemory.reset();
	stripPackedAttribute1.reset();
	stripPackedAttribute1Memory.reset();
	stripPackedAttribute2.reset();
	stripPackedAttribute2Memory.reset();
	sharedVertexPackedAttribute1.reset();
	sharedVertexPackedAttribute1Memory.reset();
	sharedVertexPackedAttribute2.reset();
	sharedVertexPackedAttribute2Memory.reset();
	sameVertexPackedAttribute1.reset();
	sameVertexPackedAttribute1Memory.reset();
	sameVertexPackedAttribute2.reset();
	sameVertexPackedAttribute2Memory.reset();
	singlePackedBuffer.reset();
	singlePackedBufferMemory.reset();
	singleMatrixUniformBuffer.reset();
	singleMatrixUniformMemory.reset();
	singlePATBuffer.reset();
	singlePATMemory.reset();
	sameMatrixAttribute.reset();
	sameMatrixAttributeMemory.reset();
	sameMatrixBuffer.reset();
	sameMatrixBufferMemory.reset();
	sameMatrixRowMajorBuffer.reset();
	sameMatrixRowMajorBufferMemory.reset();
	sameMatrix4x3Buffer.reset();
	sameMatrix4x3BufferMemory.reset();
	sameMatrix4x3RowMajorBuffer.reset();
	sameMatrix4x3RowMajorBufferMemory.reset();
	sameDMatrixBuffer.reset();
	sameDMatrixBufferMemory.reset();
	sameDMatrixStagingBuffer.reset();
	sameDMatrixStagingBufferMemory.reset();
	samePATBuffer.reset();
	samePATBufferMemory.reset();
	transformationMatrixAttribute.reset();
	transformationMatrixBuffer.reset();
	transformationMatrixAttributeMemory.reset();
	transformationMatrixBufferMemory.reset();
	normalMatrix4x3Buffer.reset();
	viewAndProjectionMatricesUniformBuffer.reset();
	materialUniformBuffer.reset();
	materialUniformBufferMemory.reset();
	materialNotPackedUniformBuffer.reset();
	materialNotPackedUniformBufferMemory.reset();
	globalLightUniformBuffer.reset();
	globalLightUniformBufferMemory.reset();
	lightUniformBuffer.reset();
	lightUniformBufferMemory.reset();
	lightNotPackedUniformBuffer.reset();
	lightNotPackedUniformBufferMemory.reset();
	allInOneLightingUniformBuffer.reset();
	allInOneLightingUniformBufferMemory.reset();
	descriptorPool.reset();
	timestampPool.reset();

	// submitNowCommandBuffer
	// that will be submitted at the end of this function
	vk::UniqueCommandPool commandPoolTransient=
		device->createCommandPoolUnique(
			vk::CommandPoolCreateInfo(
				vk::CommandPoolCreateFlagBits::eTransient|vk::CommandPoolCreateFlagBits::eResetCommandBuffer,  // flags
				graphicsQueueFamily  // queueFamilyIndex
			)
		);
	vk::UniqueCommandBuffer submitNowCommandBuffer=std::move(
		device->allocateCommandBuffersUnique(
			vk::CommandBufferAllocateInfo(
				commandPoolTransient.get(),        // commandPool
				vk::CommandBufferLevel::ePrimary,  // level
				1                                  // commandBufferCount
			)
		)[0]);
	submitNowCommandBuffer->begin(
		vk::CommandBufferBeginInfo(
			vk::CommandBufferUsageFlagBits::eOneTimeSubmit,  // flags
			nullptr  // pInheritanceInfo
		)
	);

	// images
	colorImage =
		device->createImageUnique(
			vk::ImageCreateInfo(
				vk::ImageCreateFlags(),       // flags
				vk::ImageType::e2D,           // imageType
				colorFormat,                  // format
				vk::Extent3D(framebufferExtent, 1),  // extent
				1,                            // mipLevels
				1,                            // arrayLayers
				vk::SampleCountFlagBits::e1,  // samples
				vk::ImageTiling::eOptimal,    // tiling
				vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,  // usage
				vk::SharingMode::eExclusive,  // sharingMode
				0,                            // queueFamilyIndexCount
				nullptr,                      // pQueueFamilyIndices
				vk::ImageLayout::eUndefined   // initialLayout
			)
		);
	depthImage=
		device->createImageUnique(
			vk::ImageCreateInfo(
				vk::ImageCreateFlags(),  // flags
				vk::ImageType::e2D,      // imageType
				depthFormat,             // format
				vk::Extent3D(framebufferExtent,1),  // extent
				1,                       // mipLevels
				1,                       // arrayLayers
				vk::SampleCountFlagBits::e1,  // samples
				vk::ImageTiling::eOptimal,    // tiling
				vk::ImageUsageFlagBits::eDepthStencilAttachment,  // usage
				vk::SharingMode::eExclusive,  // sharingMode
				0,                            // queueFamilyIndexCount
				nullptr,                      // pQueueFamilyIndices
				vk::ImageLayout::eUndefined   // initialLayout
			)
		);

	// image memories
	colorImageMemory=allocateMemory(colorImage.get(),vk::MemoryPropertyFlagBits::eDeviceLocal);
	device->bindImageMemory(
		colorImage.get(),  // image
		colorImageMemory.get(),  // memory
		0  // memoryOffset
	);
	depthImageMemory=allocateMemory(depthImage.get(),vk::MemoryPropertyFlagBits::eDeviceLocal);
	device->bindImageMemory(
		depthImage.get(),  // image
		depthImageMemory.get(),  // memory
		0  // memoryOffset
	);

	// image views
	colorImageView=
		device->createImageViewUnique(
			vk::ImageViewCreateInfo(
				vk::ImageViewCreateFlags(),  // flags
				colorImage.get(),            // image
				vk::ImageViewType::e2D,      // viewType
				colorFormat,                 // format
				vk::ComponentMapping(),      // components
				vk::ImageSubresourceRange(   // subresourceRange
					vk::ImageAspectFlagBits::eColor,  // aspectMask
					0,  // baseMipLevel
					1,  // levelCount
					0,  // baseArrayLayer
					1   // layerCount
				)
			)
		);
	depthImageView=
		device->createImageViewUnique(
			vk::ImageViewCreateInfo(
				vk::ImageViewCreateFlags(),  // flags
				depthImage.get(),            // image
				vk::ImageViewType::e2D,      // viewType
				depthFormat,                 // format
				vk::ComponentMapping(),      // components
				vk::ImageSubresourceRange(   // subresourceRange
					vk::ImageAspectFlagBits::eDepth,  // aspectMask
					0,  // baseMipLevel
					1,  // levelCount
					0,  // baseArrayLayer
					1   // layerCount
				)
			)
		);

	// pipeline
	auto createPipeline=
		[](vk::ShaderModule vsModule,vk::ShaderModule fsModule,vk::PipelineLayout pipelineLayout,
		   const vk::Extent2D framebufferExtent,
		   const vk::PipelineVertexInputStateCreateInfo* vertexInputState=nullptr,
		   vk::ShaderModule gsModule=nullptr,
		   const vk::PipelineInputAssemblyStateCreateInfo* inputAssemblyState=nullptr)
		   ->vk::UniquePipeline
		{
			return device->createGraphicsPipelineUnique(
				pipelineCache.get(),
				vk::GraphicsPipelineCreateInfo(
					vk::PipelineCreateFlags(),  // flags
					!gsModule?2:3,  // stageCount
					array<const vk::PipelineShaderStageCreateInfo,3>{  // pStages
						vk::PipelineShaderStageCreateInfo{
							vk::PipelineShaderStageCreateFlags(),  // flags
							vk::ShaderStageFlagBits::eVertex,      // stage
							vsModule,  // module
							"main",    // pName
							nullptr    // pSpecializationInfo
						},
						vk::PipelineShaderStageCreateInfo{
							vk::PipelineShaderStageCreateFlags(),  // flags
							vk::ShaderStageFlagBits::eFragment,    // stage
							fsModule,  // module
							"main",    // pName
							nullptr    // pSpecializationInfo
						},
						vk::PipelineShaderStageCreateInfo{
							vk::PipelineShaderStageCreateFlags(),  // flags
							vk::ShaderStageFlagBits::eGeometry,    // stage
							gsModule,  // module
							"main",    // pName
							nullptr    // pSpecializationInfo
						}
					}.data(),
					vertexInputState!=nullptr  // pVertexInputState
						?vertexInputState
						:&(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
							vk::PipelineVertexInputStateCreateFlags(),  // flags
							1,        // vertexBindingDescriptionCount
							array<const vk::VertexInputBindingDescription,1>{  // pVertexBindingDescriptions
								vk::VertexInputBindingDescription(
									0,  // binding
									4*sizeof(float),  // stride
									vk::VertexInputRate::eVertex  // inputRate
								),
							}.data(),
							1,        // vertexAttributeDescriptionCount
							array<const vk::VertexInputAttributeDescription,1>{  // pVertexAttributeDescriptions
								vk::VertexInputAttributeDescription(
									0,  // location
									0,  // binding
									vk::Format::eR32G32B32A32Sfloat,  // format
									0   // offset
								),
							}.data()
						},
					inputAssemblyState!=nullptr  // pInputAssemblyState
						?inputAssemblyState
						:&(const vk::PipelineInputAssemblyStateCreateInfo&)vk::PipelineInputAssemblyStateCreateInfo{
							vk::PipelineInputAssemblyStateCreateFlags(),  // flags
							vk::PrimitiveTopology::eTriangleList,  // topology
							VK_FALSE  // primitiveRestartEnable
						},
					nullptr, // pTessellationState
					&(const vk::PipelineViewportStateCreateInfo&)vk::PipelineViewportStateCreateInfo{  // pViewportState
						vk::PipelineViewportStateCreateFlags(),  // flags
						1,  // viewportCount
						&(const vk::Viewport&)vk::Viewport(0.f,0.f,float(framebufferExtent.width),float(framebufferExtent.height),0.f,1.f),  // pViewports
						1,  // scissorCount
						&(const vk::Rect2D&)vk::Rect2D(vk::Offset2D(0,0),framebufferExtent)  // pScissors
					},
					&(const vk::PipelineRasterizationStateCreateInfo&)vk::PipelineRasterizationStateCreateInfo{  // pRasterizationState
						vk::PipelineRasterizationStateCreateFlags(),  // flags
						VK_FALSE,  // depthClampEnable
						VK_FALSE,  // rasterizerDiscardEnable
						vk::PolygonMode::eFill,  // polygonMode
						vk::CullModeFlagBits::eNone,  // cullMode
						vk::FrontFace::eCounterClockwise,  // frontFace
						VK_FALSE,  // depthBiasEnable
						0.f,  // depthBiasConstantFactor
						0.f,  // depthBiasClamp
						0.f,  // depthBiasSlopeFactor
						1.f   // lineWidth
					},
					&(const vk::PipelineMultisampleStateCreateInfo&)vk::PipelineMultisampleStateCreateInfo{  // pMultisampleState
						vk::PipelineMultisampleStateCreateFlags(),  // flags
						vk::SampleCountFlagBits::e1,  // rasterizationSamples
						VK_FALSE,  // sampleShadingEnable
						0.f,       // minSampleShading
						nullptr,   // pSampleMask
						VK_FALSE,  // alphaToCoverageEnable
						VK_FALSE   // alphaToOneEnable
					},
					&(const vk::PipelineDepthStencilStateCreateInfo&)vk::PipelineDepthStencilStateCreateInfo{  // pDepthStencilState
						vk::PipelineDepthStencilStateCreateFlags(),  // flags
						VK_TRUE,  // depthTestEnable
						VK_TRUE,  // depthWriteEnable
						vk::CompareOp::eLess,  // depthCompareOp
						VK_FALSE,  // depthBoundsTestEnable
						VK_FALSE,  // stencilTestEnable
						vk::StencilOpState(),  // front
						vk::StencilOpState(),  // back
						0.f,  // minDepthBounds
						0.f   // maxDepthBounds
					},
					&(const vk::PipelineColorBlendStateCreateInfo&)vk::PipelineColorBlendStateCreateInfo{  // pColorBlendState
						vk::PipelineColorBlendStateCreateFlags(),  // flags
						VK_FALSE,  // logicOpEnable
						vk::LogicOp::eClear,  // logicOp
						1,  // attachmentCount
						&(const vk::PipelineColorBlendAttachmentState&)vk::PipelineColorBlendAttachmentState{  // pAttachments
							VK_FALSE,  // blendEnable
							vk::BlendFactor::eZero,  // srcColorBlendFactor
							vk::BlendFactor::eZero,  // dstColorBlendFactor
							vk::BlendOp::eAdd,       // colorBlendOp
							vk::BlendFactor::eZero,  // srcAlphaBlendFactor
							vk::BlendFactor::eZero,  // dstAlphaBlendFactor
							vk::BlendOp::eAdd,       // alphaBlendOp
							vk::ColorComponentFlagBits::eR|vk::ColorComponentFlagBits::eG|
								vk::ColorComponentFlagBits::eB|vk::ColorComponentFlagBits::eA  // colorWriteMask
						},
						array<float,4>{0.f,0.f,0.f,0.f}  // blendConstants
					},
					nullptr,  // pDynamicState
					pipelineLayout,  // layout
					renderPass.get(),  // renderPass
					0,  // subpass
					vk::Pipeline(nullptr),  // basePipelineHandle
					-1 // basePipelineIndex
				)
			).value;
		};

	const array<const vk::VertexInputBindingDescription,4> stride16AttributesBinding{
		vk::VertexInputBindingDescription(
			0,   // binding
			16,  // stride
			vk::VertexInputRate::eVertex  // inputRate
		),
		vk::VertexInputBindingDescription(
			1,   // binding
			16,  // stride
			vk::VertexInputRate::eVertex  // inputRate
		),
		vk::VertexInputBindingDescription(
			2,   // binding
			16,  // stride
			vk::VertexInputRate::eVertex  // inputRate
		),
		vk::VertexInputBindingDescription(
			3,   // binding
			16,  // stride
			vk::VertexInputRate::eVertex  // inputRate
		),
	};
	const array<const vk::VertexInputAttributeDescription,4> fourAttributesDescription{
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
			vk::Format::eR32G32B32A32Sfloat,  // format
			0   // offset
		),
		vk::VertexInputAttributeDescription(
			3,  // location
			3,  // binding
			vk::Format::eR32G32B32A32Sfloat,  // format
			0   // offset
		),
	};
	const vk::PipelineVertexInputStateCreateInfo fourAttributesInputState{
		vk::PipelineVertexInputStateCreateFlags(),  // flags
		4,  // vertexBindingDescriptionCount
		stride16AttributesBinding.data(),  // pVertexBindingDescriptions
		4,  // vertexAttributeDescriptionCount
		fourAttributesDescription.data(),  // pVertexAttributeDescriptions
	};
	const array<const vk::VertexInputAttributeDescription,2> twoPackedAttributesDescription{
		vk::VertexInputAttributeDescription(
			0,  // location
			0,  // binding
			vk::Format::eR32G32B32A32Uint,  // format
			0   // offset
		),
		vk::VertexInputAttributeDescription(
			1,  // location
			1,  // binding
			vk::Format::eR32G32B32A32Uint,  // format
			0   // offset
		),
	};
	const vk::PipelineVertexInputStateCreateInfo twoPackedAttributesInputState{
		vk::PipelineVertexInputStateCreateFlags(),  // flags
		2,  // vertexBindingDescriptionCount
		stride16AttributesBinding.data(),  // pVertexBindingDescriptions
		2,  // vertexAttributeDescriptionCount
		twoPackedAttributesDescription.data()  // pVertexAttributeDescriptions
	};
	const vk::PipelineInputAssemblyStateCreateInfo triangleStripInputAssemblyState{
		vk::PipelineInputAssemblyStateCreateFlags(),  // flags
		vk::PrimitiveTopology::eTriangleStrip,  // topology
		VK_FALSE  // primitiveRestartEnable
	};

	attributelessConstantOutputPipeline=
		createPipeline(attributelessConstantOutputVS.get(),constantColorFS.get(),simplePipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               });
	attributelessConstantOutputTriStripPipeline=
		createPipeline(attributelessConstantOutputVS.get(),constantColorFS.get(),simplePipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               },
		               nullptr,
		               &triangleStripInputAssemblyState);
	attributelessConstantOutputPrimitiveRestartPipeline=
		createPipeline(attributelessConstantOutputVS.get(),constantColorFS.get(),simplePipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               },
		               nullptr,
		               &(const vk::PipelineInputAssemblyStateCreateInfo&)vk::PipelineInputAssemblyStateCreateInfo{
			               vk::PipelineInputAssemblyStateCreateFlags(),  // flags
			               vk::PrimitiveTopology::eTriangleStrip,  // topology
			               VK_TRUE  // primitiveRestartEnable
		               });
	attributelessInputIndicesPipeline=
		createPipeline(attributelessInputIndicesVS.get(),constantColorFS.get(),simplePipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               });
	attributelessInputIndicesTriStripPipeline=
		createPipeline(attributelessInputIndicesVS.get(),constantColorFS.get(),simplePipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               },
		               nullptr,
		               &triangleStripInputAssemblyState);
	attributelessInputIndicesPrimitiveRestartPipeline=
		createPipeline(attributelessInputIndicesVS.get(),constantColorFS.get(),simplePipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               },
		               nullptr,
		               &(const vk::PipelineInputAssemblyStateCreateInfo&)vk::PipelineInputAssemblyStateCreateInfo{
			               vk::PipelineInputAssemblyStateCreateFlags(),  // flags
			               vk::PrimitiveTopology::eTriangleStrip,  // topology
			               VK_TRUE  // primitiveRestartEnable
		               });
	coordinateAttributePipeline=
		createPipeline(coordinateAttributeVS.get(),constantColorFS.get(),simplePipelineLayout.get(),framebufferExtent);
	coordinate4BufferPipeline=
		createPipeline(coordinate4BufferVS.get(),constantColorFS.get(),oneBufferPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               });
	coordinate3BufferPipeline=
		createPipeline(coordinate3BufferVS.get(),constantColorFS.get(),oneBufferPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               });
	singleMatrixUniformPipeline=
		createPipeline(singleUniformMatrixVS.get(),constantColorFS.get(),oneUniformVSPipelineLayout.get(),framebufferExtent);
	matrixAttributePipeline=
		createPipeline(matrixAttributeVS.get(),constantColorFS.get(),simplePipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               2,  // vertexBindingDescriptionCount
			               array<const vk::VertexInputBindingDescription,2>{  // pVertexBindingDescriptions
				               vk::VertexInputBindingDescription(
					               0,  // binding
					               4*sizeof(float),  // stride
					               vk::VertexInputRate::eVertex  // inputRate
				               ),
				               vk::VertexInputBindingDescription(
					               1,  // binding
					               16*sizeof(float),  // stride
					               vk::VertexInputRate::eInstance  // inputRate
				               ),
			               }.data(),
			               5,  // vertexAttributeDescriptionCount
			               array<const vk::VertexInputAttributeDescription,5>{  // pVertexAttributeDescriptions
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
					               1,  // binding
					               vk::Format::eR32G32B32A32Sfloat,  // format
					               4*sizeof(float)  // offset
				               ),
				               vk::VertexInputAttributeDescription(
					               3,  // location
					               1,  // binding
					               vk::Format::eR32G32B32A32Sfloat,  // format
					               8*sizeof(float)  // offset
				               ),
				               vk::VertexInputAttributeDescription(
					               4,  // location
					               1,  // binding
					               vk::Format::eR32G32B32A32Sfloat,  // format
					               12*sizeof(float)  // offset
				               ),
			               }.data()
		               });
	matrixBufferPipeline=
		createPipeline(matrixBufferVS.get(),constantColorFS.get(),oneBufferPipelineLayout.get(),framebufferExtent);
	twoAttributesPipeline=
		createPipeline(twoAttributesVS.get(),constantColorFS.get(),simplePipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               2,  // vertexBindingDescriptionCount
			               stride16AttributesBinding.data(),  // pVertexBindingDescriptions
			               2,  // vertexAttributeDescriptionCount
			               fourAttributesDescription.data(),  // pVertexAttributeDescriptions
		               });
	twoInterleavedAttributesPipeline=
		createPipeline(twoAttributesVS.get(),constantColorFS.get(),simplePipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               1,  // vertexBindingDescriptionCount
			               array<const vk::VertexInputBindingDescription,1>{  // pVertexBindingDescriptions
				               vk::VertexInputBindingDescription(
					               0,  // binding
					               8*sizeof(float),  // stride
					               vk::VertexInputRate::eVertex  // inputRate
				               ),
			               }.data(),
			               2,  // vertexAttributeDescriptionCount
			               array<const vk::VertexInputAttributeDescription,2>{  // pVertexAttributeDescriptions
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
			               }.data()
		               });
	twoBuffersPipeline=
		createPipeline(twoBuffersVS.get(),constantColorFS.get(),twoBuffersPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               });
	twoBuffer3Pipeline=
		createPipeline(twoBuffer3VS.get(),constantColorFS.get(),twoBuffersPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               });
	twoInterleavedBuffersPipeline=
		createPipeline(twoInterleavedBuffersVS.get(),constantColorFS.get(),oneBufferPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               });
	twoPackedAttributesPipeline=
		createPipeline(twoPackedAttributesVS.get(),constantColorFS.get(),simplePipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               2,  // vertexBindingDescriptionCount
			               stride16AttributesBinding.data(),  // pVertexBindingDescriptions
			               2,  // vertexAttributeDescriptionCount
			               array<const vk::VertexInputAttributeDescription,2>{  // pVertexAttributeDescriptions
				               vk::VertexInputAttributeDescription(
					               0,  // location
					               0,  // binding
					               vk::Format::eR32G32B32A32Uint,  // format
					               0   // offset
				               ),
				               vk::VertexInputAttributeDescription(
					               1,  // location
					               1,  // binding
					               vk::Format::eR32G32B32A32Uint,  // format
					               0   // offset
				               ),
			               }.data()
		               });
	twoPackedBuffersPipeline=
		createPipeline(twoPackedBuffersVS.get(),constantColorFS.get(),twoBuffersPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               });
	twoPackedBuffersUsingStructPipeline=
		createPipeline(twoPackedBuffersUsingStructVS.get(),constantColorFS.get(),twoBuffersPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               });
	twoPackedBuffersUsingStructSlowPipeline=
		createPipeline(twoPackedBuffersUsingStructSlowVS.get(),constantColorFS.get(),twoBuffersPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               });
	singlePackedBufferPipeline=
		createPipeline(singlePackedBufferVS.get(),constantColorFS.get(),oneBufferPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               });
	two4F32Two4U8AttributesPipeline=
		createPipeline(fourAttributesVS.get(),constantColorFS.get(),simplePipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               4,  // vertexBindingDescriptionCount
			               array<const vk::VertexInputBindingDescription,4>{  // pVertexBindingDescriptions
				               vk::VertexInputBindingDescription(
					               0,  // binding
					               4*sizeof(float),  // stride
					               vk::VertexInputRate::eVertex  // inputRate
				               ),
				               vk::VertexInputBindingDescription(
					               1,  // binding
					               4*sizeof(float),  // stride
					               vk::VertexInputRate::eVertex  // inputRate
				               ),
				               vk::VertexInputBindingDescription(
					               2,  // binding
					               4,  // stride
					               vk::VertexInputRate::eVertex  // inputRate
				               ),
				               vk::VertexInputBindingDescription(
					               3,  // binding
					               4,  // stride
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
		               });
	twoPackedAttributesAndSingleMatrixPipeline=
		createPipeline(twoPackedAttributesAndSingleMatrixVS.get(),constantColorFS.get(),oneBufferPipelineLayout.get(),framebufferExtent,
		               &twoPackedAttributesInputState);
	twoPackedAttributesAndMatrixPipeline=
		createPipeline(twoPackedAttributesAndMatrixVS.get(),constantColorFS.get(),oneBufferPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               2,  // vertexBindingDescriptionCount
			               stride16AttributesBinding.data(),  // pVertexBindingDescriptions
			               2,  // vertexAttributeDescriptionCount
			               array<const vk::VertexInputAttributeDescription,2>{  // pVertexAttributeDescriptions
				               vk::VertexInputAttributeDescription(
					               0,  // location
					               0,  // binding
					               vk::Format::eR32G32B32A32Uint,  // format
					               0   // offset
				               ),
				               vk::VertexInputAttributeDescription(
					               1,  // location
					               1,  // binding
					               vk::Format::eR32G32B32A32Uint,  // format
					               0   // offset
				               ),
			               }.data()
		               });
	twoPackedBuffersAndMatrixPipeline=
		createPipeline(twoPackedBuffersAndMatrixVS.get(),constantColorFS.get(),threeBuffersPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               });
	fourAttributesPipeline=
		createPipeline(fourAttributesVS.get(),constantColorFS.get(),simplePipelineLayout.get(),framebufferExtent,
		               &fourAttributesInputState);
	fourInterleavedAttributesPipeline=
		createPipeline(fourAttributesVS.get(),constantColorFS.get(),simplePipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
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
	fourBuffersPipeline=
		createPipeline(fourBuffersVS.get(),constantColorFS.get(),fourBuffersPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               });
	fourBuffer3Pipeline=
		createPipeline(fourBuffer3VS.get(),constantColorFS.get(),fourBuffersPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               });
	fourInterleavedBuffersPipeline=
		createPipeline(fourInterleavedBuffersVS.get(),constantColorFS.get(),oneBufferPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               });
	fourAttributesAndMatrixPipeline=
		createPipeline(fourAttributesAndMatrixVS.get(),constantColorFS.get(),oneBufferPipelineLayout.get(),framebufferExtent,
		               &fourAttributesInputState);
	if(enabledFeatures.geometryShader)
		geometryShaderPipeline=
			createPipeline(geometryShaderVS.get(),constantColorFS.get(),threeBuffersInGSPipelineLayout.get(),framebufferExtent,
			               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
				               vk::PipelineVertexInputStateCreateFlags(),  // flags
				               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
				               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
			               },
			               geometryShaderGS.get());
	if(enabledFeatures.geometryShader) {
		geometryShaderNoOutputPipeline=
			createPipeline(geometryShaderConstantOutputVS.get(),constantColorFS.get(),simplePipelineLayout.get(),framebufferExtent,
			               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
				               vk::PipelineVertexInputStateCreateFlags(),  // flags
				               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
				               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
			               },
			               geometryShaderNoOutputGS.get());
		geometryShaderConstantOutputPipeline=
			createPipeline(geometryShaderConstantOutputVS.get(),constantColorFS.get(),simplePipelineLayout.get(),framebufferExtent,
			               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
				               vk::PipelineVertexInputStateCreateFlags(),  // flags
				               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
				               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
			               },
			               geometryShaderConstantOutputGS.get());
		geometryShaderConstantOutputTwoTrianglesPipeline=
			createPipeline(geometryShaderConstantOutputVS.get(),constantColorFS.get(),simplePipelineLayout.get(),framebufferExtent,
			               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
				               vk::PipelineVertexInputStateCreateFlags(),  // flags
				               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
				               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
			               },
			               geometryShaderConstantOutputTwoTrianglesGS.get());
	}
	transformationThreeMatricesPipeline=
		createPipeline(transformationThreeMatricesVS.get(),constantColorFS.get(),bufferAndUniformPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               2,  // vertexBindingDescriptionCount
			               stride16AttributesBinding.data(),  // pVertexBindingDescriptions
			               2,  // vertexAttributeDescriptionCount
			               array<const vk::VertexInputAttributeDescription,2>{  // pVertexAttributeDescriptions
				               vk::VertexInputAttributeDescription(
					               0,  // location
					               0,  // binding
					               vk::Format::eR32G32B32A32Uint,  // format
					               0   // offset
				               ),
				               vk::VertexInputAttributeDescription(
					               1,  // location
					               1,  // binding
					               vk::Format::eR32G32B32A32Uint,  // format
					               0   // offset
				               ),
			               }.data()
		               });
	transformationFiveMatricesPipeline=
		createPipeline(transformationFiveMatricesVS.get(),constantColorFS.get(),twoBuffersAndUniformPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               2,  // vertexBindingDescriptionCount
			               stride16AttributesBinding.data(),  // pVertexBindingDescriptions
			               2,  // vertexAttributeDescriptionCount
			               array<const vk::VertexInputAttributeDescription,2>{  // pVertexAttributeDescriptions
				               vk::VertexInputAttributeDescription(
					               0,  // location
					               0,  // binding
					               vk::Format::eR32G32B32A32Uint,  // format
					               0   // offset
				               ),
				               vk::VertexInputAttributeDescription(
					               1,  // location
					               1,  // binding
					               vk::Format::eR32G32B32A32Uint,  // format
					               0   // offset
				               ),
			               }.data()
		               });
	transformationFiveMatricesPushConstantsPipeline=
		createPipeline(transformationFiveMatricesPushConstantsVS.get(),constantColorFS.get(),twoBuffersAndPushConstantsPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               2,  // vertexBindingDescriptionCount
			               stride16AttributesBinding.data(),  // pVertexBindingDescriptions
			               2,  // vertexAttributeDescriptionCount
			               array<const vk::VertexInputAttributeDescription,2>{  // pVertexAttributeDescriptions
				               vk::VertexInputAttributeDescription(
					               0,  // location
					               0,  // binding
					               vk::Format::eR32G32B32A32Uint,  // format
					               0   // offset
				               ),
				               vk::VertexInputAttributeDescription(
					               1,  // location
					               1,  // binding
					               vk::Format::eR32G32B32A32Uint,  // format
					               0   // offset
				               ),
			               }.data()
		               });
	transformationFiveMatricesSpecializationConstantsPipeline=
		createPipeline(transformationFiveMatricesSpecializationConstantsVS.get(),constantColorFS.get(),twoBuffersPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               2,  // vertexBindingDescriptionCount
			               stride16AttributesBinding.data(),  // pVertexBindingDescriptions
			               2,  // vertexAttributeDescriptionCount
			               array<const vk::VertexInputAttributeDescription,2>{  // pVertexAttributeDescriptions
				               vk::VertexInputAttributeDescription(
					               0,  // location
					               0,  // binding
					               vk::Format::eR32G32B32A32Uint,  // format
					               0   // offset
				               ),
				               vk::VertexInputAttributeDescription(
					               1,  // location
					               1,  // binding
					               vk::Format::eR32G32B32A32Uint,  // format
					               0   // offset
				               ),
			               }.data()
		               });
	transformationFiveMatricesConstantsPipeline=
		createPipeline(transformationFiveMatricesConstantsVS.get(),constantColorFS.get(),twoBuffersPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               2,  // vertexBindingDescriptionCount
			               stride16AttributesBinding.data(),  // pVertexBindingDescriptions
			               2,  // vertexAttributeDescriptionCount
			               array<const vk::VertexInputAttributeDescription,2>{  // pVertexAttributeDescriptions
				               vk::VertexInputAttributeDescription(
					               0,  // location
					               0,  // binding
					               vk::Format::eR32G32B32A32Uint,  // format
					               0   // offset
				               ),
				               vk::VertexInputAttributeDescription(
					               1,  // location
					               1,  // binding
					               vk::Format::eR32G32B32A32Uint,  // format
					               0   // offset
				               ),
			               }.data()
		               });
	if(enabledFeatures.geometryShader)
		transformationFiveMatricesUsingGSPipeline=
			createPipeline(geometryShaderVS.get(),constantColorFS.get(),fourBuffersAndUniformInGSPipelineLayout.get(),framebufferExtent,
			               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
				               vk::PipelineVertexInputStateCreateFlags(),  // flags
				               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
				               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
			               },
			               transformationFiveMatricesUsingGS.get());
	if(enabledFeatures.geometryShader)
		transformationFiveMatricesUsingGSAndAttributesPipeline=
			createPipeline(transformationFiveMatricesUsingGSAndAttributesVS.get(),constantColorFS.get(),twoBuffersAndUniformInGSPipelineLayout.get(),framebufferExtent,
			               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
				               vk::PipelineVertexInputStateCreateFlags(),  // flags
				               2,  // vertexBindingDescriptionCount
				               stride16AttributesBinding.data(),  // pVertexBindingDescriptions
				               2,  // vertexAttributeDescriptionCount
				               array<const vk::VertexInputAttributeDescription,2>{  // pVertexAttributeDescriptions
					               vk::VertexInputAttributeDescription(
						               0,  // location
						               0,  // binding
						               vk::Format::eR32G32B32A32Uint,  // format
						               0   // offset
					               ),
					               vk::VertexInputAttributeDescription(
						               1,  // location
						               1,  // binding
						               vk::Format::eR32G32B32A32Uint,  // format
						               0   // offset
					               ),
				               }.data()
			               },
			               transformationFiveMatricesUsingGSAndAttributesGS.get());
	phongTexturedFourAttributesFiveMatricesPipeline=
		createPipeline(phongTexturedFourAttributesFiveMatricesVS.get(),phongTexturedDummyFS.get(),twoBuffersAndUniformPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               4,  // vertexBindingDescriptionCount
			               array<const vk::VertexInputBindingDescription,4>{  // pVertexBindingDescriptions
				               vk::VertexInputBindingDescription(
					               0,  // binding
					               4*sizeof(float),  // stride
					               vk::VertexInputRate::eVertex  // inputRate
				               ),
				               vk::VertexInputBindingDescription(
					               1,  // binding
					               3*sizeof(float),  // stride
					               vk::VertexInputRate::eVertex  // inputRate
				               ),
				               vk::VertexInputBindingDescription(
					               2,  // binding
					               4,  // stride
					               vk::VertexInputRate::eVertex  // inputRate
				               ),
				               vk::VertexInputBindingDescription(
					               3,  // binding
					               2*sizeof(float),  // stride
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
					               1,  // binding
					               vk::Format::eR32G32B32Sfloat,  // format
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
					               vk::Format::eR32G32Sfloat,  // format
					               0   // offset
				               ),
			               }.data()
		               });
	phongTexturedFourAttributesPipeline=
		createPipeline(phongTexturedFourAttributesVS.get(),phongTexturedDummyFS.get(),bufferAndUniformPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               4,  // vertexBindingDescriptionCount
			               array<const vk::VertexInputBindingDescription,4>{  // pVertexBindingDescriptions
				               vk::VertexInputBindingDescription(
					               0,  // binding
					               4*sizeof(float),  // stride
					               vk::VertexInputRate::eVertex  // inputRate
				               ),
				               vk::VertexInputBindingDescription(
					               1,  // binding
					               3*sizeof(float),  // stride
					               vk::VertexInputRate::eVertex  // inputRate
				               ),
				               vk::VertexInputBindingDescription(
					               2,  // binding
					               4,  // stride
					               vk::VertexInputRate::eVertex  // inputRate
				               ),
				               vk::VertexInputBindingDescription(
					               3,  // binding
					               2*sizeof(float),  // stride
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
					               1,  // binding
					               vk::Format::eR32G32B32Sfloat,  // format
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
					               vk::Format::eR32G32Sfloat,  // format
					               0   // offset
				               ),
			               }.data()
		               });
	phongTexturedPipeline=
		createPipeline(phongTexturedVS.get(),phongTexturedDummyFS.get(),bufferAndUniformPipelineLayout.get(),framebufferExtent,
		               &twoPackedAttributesInputState);
	phongTexturedRowMajorPipeline=
		createPipeline(phongTexturedRowMajorVS.get(),phongTexturedDummyFS.get(),bufferAndUniformPipelineLayout.get(),framebufferExtent,
		               &twoPackedAttributesInputState);
	phongTexturedMat4x3Pipeline=
		createPipeline(phongTexturedMat4x3VS.get(),phongTexturedDummyFS.get(),bufferAndUniformPipelineLayout.get(),framebufferExtent,
		               &twoPackedAttributesInputState);
	phongTexturedMat4x3RowMajorPipeline=
		createPipeline(phongTexturedMat4x3RowMajorVS.get(),phongTexturedDummyFS.get(),bufferAndUniformPipelineLayout.get(),framebufferExtent,
		               &twoPackedAttributesInputState);
	phongTexturedQuat1Pipeline=
		createPipeline(phongTexturedQuat1VS.get(),phongTexturedDummyFS.get(),bufferAndUniformPipelineLayout.get(),framebufferExtent,
		               &twoPackedAttributesInputState);
	phongTexturedQuat2Pipeline=
		createPipeline(phongTexturedQuat2VS.get(),phongTexturedDummyFS.get(),bufferAndUniformPipelineLayout.get(),framebufferExtent,
		               &twoPackedAttributesInputState);
	phongTexturedQuat3Pipeline=
		createPipeline(phongTexturedQuat3VS.get(),phongTexturedDummyFS.get(),bufferAndUniformPipelineLayout.get(),framebufferExtent,
		               &twoPackedAttributesInputState);
	phongTexturedQuat2PrimitiveRestartPipeline=
		createPipeline(phongTexturedQuat2PrimitiveRestartVS.get(),phongTexturedDummyFS.get(),bufferAndUniformPipelineLayout.get(),framebufferExtent,
		               &twoPackedAttributesInputState,
		               nullptr,
		               &(const vk::PipelineInputAssemblyStateCreateInfo&)vk::PipelineInputAssemblyStateCreateInfo{
			               vk::PipelineInputAssemblyStateCreateFlags(),  // flags
			               vk::PrimitiveTopology::eTriangleStrip,  // topology
			               VK_TRUE  // primitiveRestartEnable
		               });
	phongTexturedSingleQuat2Pipeline=
		createPipeline(phongTexturedSingleQuat2VS.get(),phongTexturedDummyFS.get(),bufferAndUniformPipelineLayout.get(),framebufferExtent,
		               &twoPackedAttributesInputState,
		               nullptr);
	phongTexturedSingleQuat2TriStripPipeline=
		createPipeline(phongTexturedSingleQuat2VS.get(),phongTexturedDummyFS.get(),bufferAndUniformPipelineLayout.get(),framebufferExtent,
		               &twoPackedAttributesInputState,
		               nullptr,
		               &triangleStripInputAssemblyState);
	phongTexturedSingleQuat2PrimitiveRestartPipeline=
		createPipeline(phongTexturedSingleQuat2VS.get(),phongTexturedDummyFS.get(),bufferAndUniformPipelineLayout.get(),framebufferExtent,
		               &twoPackedAttributesInputState,
		               nullptr,
		               &(const vk::PipelineInputAssemblyStateCreateInfo&)vk::PipelineInputAssemblyStateCreateInfo{
			               vk::PipelineInputAssemblyStateCreateFlags(),  // flags
			               vk::PrimitiveTopology::eTriangleStrip,  // topology
			               VK_TRUE  // primitiveRestartEnable
		               });
	if(enabledFeatures.shaderFloat64)
		phongTexturedDMatricesOnlyInputPipeline=
			createPipeline(phongTexturedDMatricesOnlyInputVS.get(),phongTexturedDummyFS.get(),bufferAndUniformPipelineLayout.get(),framebufferExtent,
			               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
				               vk::PipelineVertexInputStateCreateFlags(),  // flags
				               2,  // vertexBindingDescriptionCount
				               stride16AttributesBinding.data(),  // pVertexBindingDescriptions
				               2,  // vertexAttributeDescriptionCount
				               array<const vk::VertexInputAttributeDescription,2>{  // pVertexAttributeDescriptions
					               vk::VertexInputAttributeDescription(
						               0,  // location
						               0,  // binding
						               vk::Format::eR32G32B32A32Uint,  // format
						               0   // offset
					               ),
					               vk::VertexInputAttributeDescription(
						               1,  // location
						               1,  // binding
						               vk::Format::eR32G32B32A32Uint,  // format
						               0   // offset
					               ),
				               }.data()
			               });
	if(enabledFeatures.shaderFloat64)
		phongTexturedDMatricesPipeline=
			createPipeline(phongTexturedDMatricesVS.get(),phongTexturedDummyFS.get(),bufferAndUniformPipelineLayout.get(),framebufferExtent,
			               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
				               vk::PipelineVertexInputStateCreateFlags(),  // flags
				               2,  // vertexBindingDescriptionCount
				               stride16AttributesBinding.data(),  // pVertexBindingDescriptions
				               2,  // vertexAttributeDescriptionCount
				               array<const vk::VertexInputAttributeDescription,2>{  // pVertexAttributeDescriptions
					               vk::VertexInputAttributeDescription(
						               0,  // location
						               0,  // binding
						               vk::Format::eR32G32B32A32Uint,  // format
						               0   // offset
					               ),
					               vk::VertexInputAttributeDescription(
						               1,  // location
						               1,  // binding
						               vk::Format::eR32G32B32A32Uint,  // format
						               0   // offset
					               ),
				               }.data()
			               });
	if(enabledFeatures.shaderFloat64)
		phongTexturedDMatricesDVerticesPipeline=
			createPipeline(phongTexturedDMatricesDVerticesVS.get(),phongTexturedDummyFS.get(),bufferAndUniformPipelineLayout.get(),framebufferExtent,
			               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
				               vk::PipelineVertexInputStateCreateFlags(),  // flags
				               3,  // vertexBindingDescriptionCount
				               stride16AttributesBinding.data(),  // pVertexBindingDescriptions
				               3,  // vertexAttributeDescriptionCount
				               array<const vk::VertexInputAttributeDescription,3>{  // pVertexAttributeDescriptions
					               vk::VertexInputAttributeDescription(
						               0,  // location
						               0,  // binding
						               vk::Format::eR32G32B32A32Uint,  // format
						               0   // offset
					               ),
					               vk::VertexInputAttributeDescription(
						               1,  // location
						               1,  // binding
						               vk::Format::eR32G32B32A32Uint,  // format
						               0   // offset
					               ),
					               vk::VertexInputAttributeDescription(
						               2,  // location
						               2,  // binding
						               vk::Format::eR32G32B32A32Uint,  // format
						               0   // offset
					               ),
				               }.data()
			               });
	if(enabledFeatures.shaderFloat64 && enabledFeatures.geometryShader)
		phongTexturedInGSDMatricesDVerticesPipeline=
			createPipeline(phongTexturedInGSDMatricesDVerticesVS.get(),phongTexturedDummyFS.get(),bufferAndUniformInGSPipelineLayout.get(),framebufferExtent,
			               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
				               vk::PipelineVertexInputStateCreateFlags(),  // flags
				               3,  // vertexBindingDescriptionCount
				               stride16AttributesBinding.data(),  // pVertexBindingDescriptions
				               3,  // vertexAttributeDescriptionCount
				               array<const vk::VertexInputAttributeDescription,3>{  // pVertexAttributeDescriptions
					               vk::VertexInputAttributeDescription(
						               0,  // location
						               0,  // binding
						               vk::Format::eR32G32B32A32Uint,  // format
						               0   // offset
					               ),
					               vk::VertexInputAttributeDescription(
						               1,  // location
						               1,  // binding
						               vk::Format::eR32G32B32A32Uint,  // format
						               0   // offset
					               ),
					               vk::VertexInputAttributeDescription(
						               2,  // location
						               2,  // binding
						               vk::Format::eR32G32B32A32Uint,  // format
						               0   // offset
					               ),
				               }.data()
			               },
			               phongTexturedInGSDMatricesDVerticesGS.get());
	fillrateContantColorPipeline=
		createPipeline(fullscreenQuadVS.get(),constantColorFS.get(),simplePipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               },
		               nullptr,
		               &triangleStripInputAssemblyState);
	fillrateFourSmoothInterpolatorsPipeline=
		createPipeline(fullscreenQuadFourInterpolatorsVS.get(),fullscreenQuadFourSmoothInterpolatorsFS.get(),simplePipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               },
		               nullptr,
		               &triangleStripInputAssemblyState);
	fillrateFourFlatInterpolatorsPipeline=
		createPipeline(fullscreenQuadFourInterpolatorsVS.get(),fullscreenQuadFourFlatInterpolatorsFS.get(),simplePipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               },
		               nullptr,
		               &triangleStripInputAssemblyState);
	fillrateTexturedPhongInterpolatorsPipeline=
		createPipeline(fullscreenQuadTexturedPhongInterpolatorsVS.get(),fullscreenQuadTexturedPhongInterpolatorsFS.get(),simplePipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               },
		               nullptr,
		               &triangleStripInputAssemblyState);
	fillrateTexturedPhongPipeline=
		createPipeline(fullscreenQuadTexturedPhongInterpolatorsVS.get(),phongTexturedFS.get(),phongTexturedPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               },
		               nullptr,
		               &triangleStripInputAssemblyState);
	fillrateTexturedPhongNotPackedPipeline=
		createPipeline(fullscreenQuadTexturedPhongInterpolatorsVS.get(),phongTexturedNotPackedFS.get(),phongTexturedPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               },
		               nullptr,
		               &triangleStripInputAssemblyState);
	fillrateUniformColor4fPipeline=
		createPipeline(fullscreenQuadVS.get(),uniformColor4fFS.get(),oneUniformFSPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               },
		               nullptr,
		               &triangleStripInputAssemblyState);
	fillrateUniformColor4bPipeline=
		createPipeline(fullscreenQuadVS.get(),uniformColor4bFS.get(),oneUniformFSPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               },
		               nullptr,
		               &triangleStripInputAssemblyState);
	phongNoSpecularPipeline=
		createPipeline(fullscreenQuadTwoVec3InterpolatorsVS.get(),phongNoSpecularFS.get(),threeUniformFSPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               },
		               nullptr,
		               &triangleStripInputAssemblyState);
	phongNoSpecularSingleUniformPipeline=
		createPipeline(fullscreenQuadTwoVec3InterpolatorsVS.get(),phongNoSpecularSingleUniformFS.get(),oneUniformFSPipelineLayout.get(),framebufferExtent,
		               &(const vk::PipelineVertexInputStateCreateInfo&)vk::PipelineVertexInputStateCreateInfo{
			               vk::PipelineVertexInputStateCreateFlags(),  // flags
			               0,nullptr,  // vertexBindingDescriptionCount,pVertexBindingDescriptions
			               0,nullptr   // vertexAttributeDescriptionCount,pVertexAttributeDescriptions
		               },
		               nullptr,
		               &triangleStripInputAssemblyState);

	// framebuffer
	framebuffer=
		device->createFramebufferUnique(
			vk::FramebufferCreateInfo(
				vk::FramebufferCreateFlags(),  // flags
				renderPass.get(),              // renderPass
				2,  // attachmentCount
				array<vk::ImageView,2>{  // pAttachments
					colorImageView.get(),
					depthImageView.get()
				}.data(),
				framebufferExtent.width,   // width
				framebufferExtent.height,  // height
				1  // layers
			)
		);

	// coordinate attributes and storage buffers
	struct BindInfo {
		bool sparseAllowed;
		vk::Buffer buffer;
		vk::DeviceMemory memory;
		size_t size;
		BindInfo(bool sparseAllowed_,vk::Buffer buffer_,vk::DeviceMemory memory_,size_t size_) : sparseAllowed(sparseAllowed_), buffer(buffer_), memory(memory_), size(size_)  {}
	};
	typedef vector<BindInfo> BindInfoList;
	BindInfoList bindInfoList;
	auto createBuffer=
		[](vk::UniqueBuffer& buffer,vk::UniqueDeviceMemory& memory,size_t size,bool sparseAllowed,vk::BufferUsageFlags usage,BindInfoList& bindInfoList) {
			buffer=
				device->createBufferUnique(
					vk::BufferCreateInfo(
						(sparseAllowed) ? bufferCreateFlags : vk::BufferCreateFlags(),  // flags
						(sparseAllowed) ? size*bufferSizeMultiplier : size,    // size
						usage,                        // usage
						vk::SharingMode::eExclusive,  // sharingMode
						0,                            // queueFamilyIndexCount
						nullptr                       // pQueueFamilyIndices
					)
				);
			size_t allocatedSize;
			tie(memory,allocatedSize)=allocateMemory(buffer.get(),vk::MemoryPropertyFlagBits::eDeviceLocal,bufferSizeMultiplier);
			bindInfoList.emplace_back(sparseAllowed,buffer.get(),memory.get(),allocatedSize);
		};
	auto startTime=chrono::high_resolution_clock::now();
	size_t coordinate4BufferSize=getBufferSize(numTriangles,true); // ~48MB
	size_t coordinate3BufferSize=getBufferSize(numTriangles,false); // ~36MB
	size_t normalBufferSize=size_t(numTriangles)*3*3*sizeof(float); // ~36MB
	size_t colorBufferSize=size_t(numTriangles)*3*4;
	size_t texCoordBufferSize=size_t(numTriangles)*3*2*sizeof(float);
	size_t vec4u8BufferSize=size_t(numTriangles)*3*4;
	size_t packedDataBufferSize=size_t(numTriangles)*3*16; // ~48MB
	size_t twoInterleavedBuffersSize=size_t(numTriangles)*3*32; // ~96MB
	size_t fourInterleavedBuffersSize=size_t(numTriangles)*3*64; // ~192MB
	size_t indexBufferSize=size_t(numTriangles)*3*4;
	size_t primitiveRestartIndexBufferSize=size_t(numTriangles)*4*4;
	size_t stripIndexBufferSize=getIndexBufferSize(numTriangles/maxTriStripLength,maxTriStripLength);
	size_t stripPrimitiveRestart3IndexBufferSize=getStripIndexPrimitiveRestartBufferSize(numTriangles/1, 1);
	size_t stripPrimitiveRestart4IndexBufferSize=getStripIndexPrimitiveRestartBufferSize(numTriangles/2, 2);
	size_t stripPrimitiveRestart7IndexBufferSize=getStripIndexPrimitiveRestartBufferSize(numTriangles/5, 5);
	size_t stripPrimitiveRestart10IndexBufferSize=getStripIndexPrimitiveRestartBufferSize(numTriangles/8, 8);
	size_t stripPrimitiveRestart1002IndexBufferSize=getStripIndexPrimitiveRestartBufferSize(numTriangles/1000, 1000);
	size_t primitiveRestartMinusOne2IndexBufferSize=getStripIndexPrimitiveRestartBufferSize(numTriangles/1, 1, 2);
	size_t primitiveRestartMinusOne5IndexBufferSize=getStripIndexPrimitiveRestartBufferSize(numTriangles/1, 1, 5);
	size_t minusOneIndexBufferSize=size_t(numTriangles)*sizeof(uint32_t);
	size_t zeroIndexBufferSize=size_t(numTriangles)*sizeof(uint32_t)*3;
	size_t plusOneIndexBufferSize=size_t(numTriangles)*sizeof(uint32_t)*3;
	size_t stripPackedDataBufferSize=getBufferSize(numTriangles/maxTriStripLength,maxTriStripLength,true);
	size_t sharedVertexPackedDataBufferSize=getBufferSizeForSharedVertexTriangles(numTriangles/maxTriStripLength,maxTriStripLength,true);
	size_t sameVertexPackedDataBufferSize=size_t(numTriangles)*3*16; // ~48MB
	createBuffer(coordinate4Attribute,coordinate4AttributeMemory,coordinate4BufferSize,true,vk::BufferUsageFlagBits::eVertexBuffer |vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(coordinate4Buffer,   coordinate4BufferMemory,   coordinate4BufferSize,true,vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(coordinate3Attribute,coordinate3AttributeMemory,coordinate3BufferSize,true,vk::BufferUsageFlagBits::eVertexBuffer |vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(coordinate3Buffer,   coordinate3BufferMemory,   coordinate3BufferSize,true,vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(normalAttribute,    normalAttributeMemory,    normalBufferSize,    true,vk::BufferUsageFlagBits::eVertexBuffer |vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(colorAttribute,     colorAttributeMemory,     colorBufferSize,     true,vk::BufferUsageFlagBits::eVertexBuffer |vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(texCoordAttribute,  texCoordAttributeMemory,  texCoordBufferSize,  true,vk::BufferUsageFlagBits::eVertexBuffer |vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	for(size_t i=0,c=vec4Attributes.size(); i<c; i++)
		createBuffer(vec4Attributes[i],vec4AttributeMemory[i],coordinate4BufferSize,true,vk::BufferUsageFlagBits::eVertexBuffer |vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	for(size_t i=0,c=vec4u8Attributes.size(); i<c; i++)
		createBuffer(vec4u8Attributes[i],vec4u8AttributeMemory[i],vec4u8BufferSize, true,vk::BufferUsageFlagBits::eVertexBuffer |vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	for(size_t i=0,c=vec4Buffers.size(); i<c; i++)
		createBuffer(vec4Buffers[i],vec4BufferMemory[i],      coordinate4BufferSize,true,vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	for(size_t i=0,c=vec3Buffers.size(); i<c; i++)
		createBuffer(vec3Buffers[i],vec3BufferMemory[i],      coordinate3BufferSize,true,vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(packedAttribute1,  packedAttribute1Memory,  packedDataBufferSize,  true,vk::BufferUsageFlagBits::eVertexBuffer |vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(packedAttribute2,  packedAttribute2Memory,  packedDataBufferSize,  true,vk::BufferUsageFlagBits::eVertexBuffer |vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(twoInterleavedAttributes, twoInterleavedAttributesMemory, twoInterleavedBuffersSize, true,vk::BufferUsageFlagBits::eVertexBuffer |vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(twoInterleavedBuffers,    twoInterleavedBuffersMemory,    twoInterleavedBuffersSize, true,vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(fourInterleavedAttributes,fourInterleavedAttributesMemory,fourInterleavedBuffersSize,true,vk::BufferUsageFlagBits::eVertexBuffer |vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(fourInterleavedBuffers,   fourInterleavedBuffersMemory,   fourInterleavedBuffersSize,true,vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(packedBuffer1,     packedBuffer1Memory,     packedDataBufferSize,  true,vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(packedBuffer2,     packedBuffer2Memory,     packedDataBufferSize,  true,vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(singlePackedBuffer,singlePackedBufferMemory,packedDataBufferSize*2,true,vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(packedDAttribute1, packedDAttribute1Memory, packedDataBufferSize,  true,vk::BufferUsageFlagBits::eVertexBuffer |vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(packedDAttribute2, packedDAttribute2Memory, packedDataBufferSize,  true,vk::BufferUsageFlagBits::eVertexBuffer |vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(packedDAttribute3, packedDAttribute3Memory, packedDataBufferSize,  true,vk::BufferUsageFlagBits::eVertexBuffer |vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(indexBuffer,       indexBufferMemory,       indexBufferSize,       true,vk::BufferUsageFlagBits::eIndexBuffer  |vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(primitiveRestartIndexBuffer,         primitiveRestartIndexBufferMemory,         primitiveRestartIndexBufferSize,         true,vk::BufferUsageFlagBits::eIndexBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(stripIndexBuffer,                    stripIndexBufferMemory,                    stripIndexBufferSize,                    true,vk::BufferUsageFlagBits::eIndexBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(stripPrimitiveRestart3IndexBuffer,   stripPrimitiveRestart3IndexBufferMemory,   stripPrimitiveRestart3IndexBufferSize,   true,vk::BufferUsageFlagBits::eIndexBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(stripPrimitiveRestart4IndexBuffer,   stripPrimitiveRestart4IndexBufferMemory,   stripPrimitiveRestart4IndexBufferSize,   true,vk::BufferUsageFlagBits::eIndexBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(stripPrimitiveRestart7IndexBuffer,   stripPrimitiveRestart7IndexBufferMemory,   stripPrimitiveRestart7IndexBufferSize,   true,vk::BufferUsageFlagBits::eIndexBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(stripPrimitiveRestart10IndexBuffer,  stripPrimitiveRestart10IndexBufferMemory,  stripPrimitiveRestart10IndexBufferSize,  true,vk::BufferUsageFlagBits::eIndexBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(stripPrimitiveRestart1002IndexBuffer,stripPrimitiveRestart1002IndexBufferMemory,stripPrimitiveRestart1002IndexBufferSize,true,vk::BufferUsageFlagBits::eIndexBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(primitiveRestartMinusOne2IndexBuffer,primitiveRestartMinusOne2IndexBufferMemory,primitiveRestartMinusOne2IndexBufferSize,true,vk::BufferUsageFlagBits::eIndexBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(primitiveRestartMinusOne5IndexBuffer,primitiveRestartMinusOne5IndexBufferMemory,primitiveRestartMinusOne5IndexBufferSize,true,vk::BufferUsageFlagBits::eIndexBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(minusOneIndexBuffer,         minusOneIndexBufferMemory,         minusOneIndexBufferSize,         true,vk::BufferUsageFlagBits::eIndexBuffer |vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(zeroIndexBuffer,             zeroIndexBufferMemory,             zeroIndexBufferSize,             true,vk::BufferUsageFlagBits::eIndexBuffer |vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(plusOneIndexBuffer,          plusOneIndexBufferMemory,          plusOneIndexBufferSize,          true,vk::BufferUsageFlagBits::eIndexBuffer |vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(stripPackedAttribute1,       stripPackedAttribute1Memory,       stripPackedDataBufferSize,       true,vk::BufferUsageFlagBits::eVertexBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(stripPackedAttribute2,       stripPackedAttribute2Memory,       stripPackedDataBufferSize,       true,vk::BufferUsageFlagBits::eVertexBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(sharedVertexPackedAttribute1,sharedVertexPackedAttribute1Memory,sharedVertexPackedDataBufferSize,true,vk::BufferUsageFlagBits::eVertexBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(sharedVertexPackedAttribute2,sharedVertexPackedAttribute2Memory,sharedVertexPackedDataBufferSize,true,vk::BufferUsageFlagBits::eVertexBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(sameVertexPackedAttribute1,  sameVertexPackedAttribute1Memory,  sameVertexPackedDataBufferSize,  true,vk::BufferUsageFlagBits::eVertexBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(sameVertexPackedAttribute2,  sameVertexPackedAttribute2Memory,  sameVertexPackedDataBufferSize,  true,vk::BufferUsageFlagBits::eVertexBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	double totalMeasurementTime=chrono::duration<double>(chrono::high_resolution_clock::now()-startTime).count();
	if(debug) {
		cout<<"First buffer and memory set"<<endl;
		cout<<"   Creation time: "<<totalMeasurementTime*1000<<"ms"<<endl;
	}

	if(sparseMode==SPARSE_NONE)
	{
		size_t totalMemorySize=0;
		auto startTime=chrono::high_resolution_clock::now();
		for(auto &bindInfo : bindInfoList) {
			device->bindBufferMemory(
				bindInfo.buffer,  // buffer
				bindInfo.memory,  // memory
				0  // memoryOffset
			);
			totalMemorySize+=bindInfo.size;
		}
		if(debug) {
			double totalMeasurementTime=chrono::duration<double>(chrono::high_resolution_clock::now()-startTime).count();
			cout<<"   Standard binding: "<<totalMeasurementTime*1000<<"ms"<<endl;
			cout<<"   (total amount of memory: "<<((totalMemorySize+512)/1024+512)/1024<<"MiB, number of memory allocations: "<<bindInfoList.size()<<")"<<endl;
		}
		bindInfoList.clear();
	}
	else
	{
		vector<vk::SparseBufferMemoryBindInfo> bufferBinds;
		vector<vk::SparseMemoryBind> memoryBinds;
		bufferBinds.reserve(bindInfoList.size());
		memoryBinds.reserve(bindInfoList.size());
		size_t numBlocks=0;
		size_t numMemObjs=0;
		for(auto &bindInfo : bindInfoList) {
			if(!bindInfo.sparseAllowed) {
				device->bindBufferMemory(
					bindInfo.buffer,  // buffer
					bindInfo.memory,  // memory
					0  // memoryOffset
				);
				continue;
			}
			auto& r=memoryBinds.emplace_back(
				0,  // resourceOffset
				bindInfo.size,  // size
				bindInfo.memory,  // memory
				0,  // memoryOffset
				vk::SparseMemoryBindFlags()  // flags
			);
			bufferBinds.emplace_back(
				bindInfo.buffer,  // buffer
				1,  // bindCount
				&r  // pBinds
			);
			numBlocks+=bindInfo.size/sparseBlockSize;
			numMemObjs++;
		}
		bindInfoList.clear();
		auto startTime=chrono::high_resolution_clock::now();
		sparseQueue.bindSparse(
			vk::BindSparseInfo(
				nullptr,  // waitSemaphores
				bufferBinds,  // bufferBinds
				nullptr,  // imageOpaqueBinds
				nullptr,  // imageBinds
				nullptr  // signalSemaphores
			),
			vk::Fence()
		);
		sparseQueue.waitIdle();
		if(debug) {
			double totalMeasurementTime=chrono::duration<double>(chrono::high_resolution_clock::now()-startTime).count();
			cout<<"   Sparse binding: "<<totalMeasurementTime*1000<<"ms\n"
			    <<"   Binding time of a memory block: "<<totalMeasurementTime/numBlocks*1e6<<"us"<<endl;
			cout<<"   (number of blocks: "<<numBlocks<<", number of memory objects: "<<numMemObjs<<")"<<endl;
		}
	}

	// staging buffer struct
	struct StagingBuffer {
		vk::UniqueBuffer buffer;
		vk::UniqueDeviceMemory memory;
		void* ptr = nullptr;
		void* map()  { if(!ptr) ptr=device->mapMemory(memory.get(),0,VK_WHOLE_SIZE,vk::MemoryMapFlags()); return ptr; }
		void unmap()  { if(ptr){ device->unmapMemory(memory.get()); ptr=nullptr; } }
		void reset()  { buffer.reset(); memory.reset(); }
		~StagingBuffer() { unmap(); }
		StagingBuffer() = default;
		StagingBuffer(StagingBuffer&& s) = default;
		StagingBuffer(size_t bufferSize)
		{
			buffer=
				device->createBufferUnique(
					vk::BufferCreateInfo(
						vk::BufferCreateFlags(),      // flags
						bufferSize,                   // size
						vk::BufferUsageFlagBits::eTransferSrc,  // usage
						vk::SharingMode::eExclusive,  // sharingMode
						0,                            // queueFamilyIndexCount
						nullptr                       // pQueueFamilyIndices
					)
				);
			tie(memory,ignore)=
				allocateMemory(buffer.get(),
				               vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent|vk::MemoryPropertyFlagBits::eHostCached,1);
			device->bindBufferMemory(
				buffer.get(),  // buffer
				memory.get(),  // memory
				0  // memoryOffset
			);
		}
	};

	// attribute staging buffers
	StagingBuffer coordinate4StagingBuffer(coordinate4BufferSize);
	StagingBuffer coordinate3StagingBuffer(coordinate3BufferSize);
	StagingBuffer normalStagingBuffer(normalBufferSize);
	StagingBuffer colorStagingBuffer(colorBufferSize);
	StagingBuffer texCoordStagingBuffer(texCoordBufferSize);
	StagingBuffer vec4Attribute1StagingBuffer(coordinate4BufferSize);
	StagingBuffer vec4Attribute2StagingBuffer(coordinate4BufferSize);
	StagingBuffer vec4u8AttributeStagingBuffer(vec4u8BufferSize);
	StagingBuffer packedAttribute1StagingBuffer(packedDataBufferSize);
	StagingBuffer packedAttribute2StagingBuffer(packedDataBufferSize);
	StagingBuffer twoInterleavedBuffersStagingBuffer(twoInterleavedBuffersSize);
	StagingBuffer fourInterleavedBuffersStagingBuffer(fourInterleavedBuffersSize);
	StagingBuffer singlePackedBufferStagingBuffer(packedDataBufferSize*2);
	StagingBuffer packedDAttribute1StagingBuffer(packedDataBufferSize);
	StagingBuffer packedDAttribute2StagingBuffer(packedDataBufferSize);
	StagingBuffer packedDAttribute3StagingBuffer(packedDataBufferSize);
	StagingBuffer indexStagingBuffer(indexBufferSize);
	StagingBuffer primitiveRestartIndexStagingBuffer(primitiveRestartIndexBufferSize);
	StagingBuffer stripIndexStagingBuffer(stripIndexBufferSize);
	StagingBuffer stripPrimitiveRestart3IndexStagingBuffer(stripPrimitiveRestart3IndexBufferSize);
	StagingBuffer stripPrimitiveRestart4IndexStagingBuffer(stripPrimitiveRestart4IndexBufferSize);
	StagingBuffer stripPrimitiveRestart7IndexStagingBuffer(stripPrimitiveRestart7IndexBufferSize);
	StagingBuffer stripPrimitiveRestart10IndexStagingBuffer(stripPrimitiveRestart10IndexBufferSize);
	StagingBuffer stripPrimitiveRestart1002IndexStagingBuffer(stripPrimitiveRestart1002IndexBufferSize);
	StagingBuffer primitiveRestartMinusOne2IndexStagingBuffer(primitiveRestartMinusOne2IndexBufferSize);
	StagingBuffer primitiveRestartMinusOne5IndexStagingBuffer(primitiveRestartMinusOne5IndexBufferSize);
	StagingBuffer minusOneIndexStagingBuffer(minusOneIndexBufferSize);
	StagingBuffer zeroIndexStagingBuffer(zeroIndexBufferSize);
	StagingBuffer plusOneIndexStagingBuffer(plusOneIndexBufferSize);
	StagingBuffer stripPackedAttribute1StagingBuffer(stripPackedDataBufferSize);
	StagingBuffer stripPackedAttribute2StagingBuffer(stripPackedDataBufferSize);
	StagingBuffer sharedVertexPackedAttribute1StagingBuffer(sharedVertexPackedDataBufferSize);
	StagingBuffer sharedVertexPackedAttribute2StagingBuffer(sharedVertexPackedDataBufferSize);
	StagingBuffer sameVertexPackedAttribute1StagingBuffer(sameVertexPackedDataBufferSize);
	StagingBuffer sameVertexPackedAttribute2StagingBuffer(sameVertexPackedDataBufferSize);
#if 0
	cout<<"First buffer set memory requirements: "<<
		   (2*coordinate4BufferSize+2*coordinate3BufferSize+
			normalBufferSize+colorBufferSize+texCoordBufferSize+
			6*coordinate4BufferSize+3*coordinate3BufferSize+vec4u8BufferSize*vec4u8Attributes.size()+
			4*packedDataBufferSize+2*twoInterleavedBuffersSize+2*fourInterleavedBuffersSize+5*packedDataBufferSize+
			indexBufferSize+primitiveRestartIndexBufferSize+stripIndexBufferSize+
			stripPrimitiveRestartIndexBufferSize+stripPrimitiveRestart3IndexBufferSize+
			stripPrimitiveRestart4IndexBufferSize+stripPrimitiveRestart7IndexBufferSize+
			stripPrimitiveRestart10IndexBufferSize+
			primitiveRestartMinusOne2IndexBufferSize+primitiveRestartMinusOne5IndexBufferSize+
			minusOneIndexBufferSize+zeroIndexBufferSize+plusOneIndexBufferSize+
			stripPackedDataBufferSize+stripPackedDataBufferSize+sharedVertexPackedDataBufferSize+
			sharedVertexPackedDataBufferSize+sameVertexPackedDataBufferSize+
			sameVertexPackedDataBufferSize)/1024/1024<<"MiB"<<endl;
#endif

	// vec3 coordinates
	float* coords=reinterpret_cast<float*>(coordinate3StagingBuffer.map());
	generateCoordinates(
		coords,numTriangles,triangleSize,
		framebufferExtent.width,framebufferExtent.height,false,
		2./framebufferExtent.width,2./framebufferExtent.height,-1.,-1.);
	coordinate3StagingBuffer.unmap();

	// vec4 coordinates
	coords=reinterpret_cast<float*>(coordinate4StagingBuffer.map());
	generateCoordinates(
		coords,numTriangles,triangleSize,
		framebufferExtent.width,framebufferExtent.height,true,
		2./framebufferExtent.width,2./framebufferExtent.height,-1.,-1.);

	// 2xvec4 in one buffer
	float* pfloat=reinterpret_cast<float*>(twoInterleavedBuffersStagingBuffer.map());
	for(size_t i=0; i<numTriangles*3; i++) {
		pfloat[i*8+0]=coords[i*4+0];
		pfloat[i*8+1]=coords[i*4+1];
		pfloat[i*8+2]=coords[i*4+2];
		pfloat[i*8+3]=coords[i*4+3];
		pfloat[i*8+4]=-2.f;
		pfloat[i*8+5]=-2.f;
		pfloat[i*8+6]=-2.f;
		pfloat[i*8+7]=-2.f;
	}
	twoInterleavedBuffersStagingBuffer.unmap();

	// 4xvec4 in one buffer
	pfloat=reinterpret_cast<float*>(fourInterleavedBuffersStagingBuffer.map());
	for(size_t i=0; i<numTriangles*3; i++) {
		pfloat[i*16+0]=coords[i*4+0];
		pfloat[i*16+1]=coords[i*4+1];
		pfloat[i*16+2]=coords[i*4+2];
		pfloat[i*16+3]=coords[i*4+3];
		pfloat[i*16+4]=-2.f;
		pfloat[i*16+5]=-2.f;
		pfloat[i*16+6]=-2.f;
		pfloat[i*16+7]=-2.f;
		pfloat[i*16+8]=-2.f;
		pfloat[i*16+9]=-2.f;
		pfloat[i*16+10]=-2.f;
		pfloat[i*16+11]=-2.f;
		pfloat[i*16+12]=4.f;
		pfloat[i*16+13]=4.f;
		pfloat[i*16+14]=4.f;
		pfloat[i*16+15]=4.f;
	}
	fourInterleavedBuffersStagingBuffer.unmap();
	coordinate4StagingBuffer.unmap();

	// vec3 normals, set to (1,1,1)
	pfloat=reinterpret_cast<float*>(normalStagingBuffer.map());
	for(size_t i=0,c=size_t(numTriangles)*3*3; i<c; i++)
		pfloat[i]=1.f;
	normalStagingBuffer.unmap();

	// uint32_t color, set to 0x0055AAFF
	uint32_t* puint=reinterpret_cast<uint32_t*>(colorStagingBuffer.map());
	for(size_t i=0,c=size_t(numTriangles)*3; i<c; i++)
		puint[i]=0x0055AAFF;
	colorStagingBuffer.unmap();

	// vec2 texCoords, set to ((0,0),(1,0),(0,1))
	pfloat=reinterpret_cast<float*>(texCoordStagingBuffer.map());
	for(size_t i=0,c=size_t(numTriangles)*3*2; i<c; ) {
		pfloat[i++]=0.f;
		pfloat[i++]=0.f;
		pfloat[i++]=1.f;
		pfloat[i++]=0.f;
		pfloat[i++]=0.f;
		pfloat[i++]=1.f;
	}
	texCoordStagingBuffer.unmap();

	// vec4 attribute1, set to (-2,-2,-2,-2)
	pfloat=reinterpret_cast<float*>(vec4Attribute1StagingBuffer.map());
	for(size_t i=0,c=size_t(numTriangles)*3*4; i<c; i++)
		pfloat[i]=-2.f;
	vec4Attribute1StagingBuffer.unmap();

	// vec4 attribute2, set to (4,4,4,4)
	pfloat=reinterpret_cast<float*>(vec4Attribute2StagingBuffer.map());
	for(size_t i=0,c=size_t(numTriangles)*3*4; i<c; i++)
		pfloat[i]=4.f;
	vec4Attribute2StagingBuffer.unmap();

	// vec4u8 attribute, set to 0xFFFFFFFF
	puint=reinterpret_cast<uint32_t*>(vec4u8AttributeStagingBuffer.map());
	for(size_t i=0,c=size_t(numTriangles)*3; i<c; i++)
		puint[i]=0xFFFFFFFF;
	vec4u8AttributeStagingBuffer.unmap();

	// packedAttribute1
	generateCoordinates(
		reinterpret_cast<float*>(packedAttribute1StagingBuffer.map()),numTriangles,triangleSize,
		framebufferExtent.width,framebufferExtent.height,true,
		2./framebufferExtent.width,2./framebufferExtent.height,-1.,-1.);
	for(size_t i=3,e=size_t(numTriangles)*3*4; i<e; i+=4)
		reinterpret_cast<uint32_t*>(packedAttribute1StagingBuffer.ptr)[i]=0x3c003c00; // two half-floats, both set to one

	// packedAttribute2
	puint=reinterpret_cast<uint32_t*>(packedAttribute2StagingBuffer.map());
	for(size_t i=0,e=size_t(numTriangles)*3*4; i<e; ) {
		puint[i++]=0x3f800000;  // texture U (float), one (1.f is 0x3f800000, 0.f is 0x00000000)
		puint[i++]=0x3f800000;  // texture V (float), one
		puint[i++]=0x3c003c00;  // normalX+Y (2x half), two times one (1.f in half-float is 0x3c00, 0.f is 0x0000)
		puint[i++]=0xffffffff;  // color
		puint[i++]=0x3f800000;  // texture U (float), one
		puint[i++]=0x3f800000;  // texture V (float), one
		puint[i++]=0x3c003c00;  // normalX+Y (2x half), two times one
		puint[i++]=0xffffffff;  // color
		puint[i++]=0x3f800000;  // texture U (float), one
		puint[i++]=0x3f800000;  // texture V (float), one
		puint[i++]=0x3c003c00;  // normalX+Y (2x half), two times one
		puint[i++]=0xffffffff;  // color
	}

	// singlePackedBuffer
	singlePackedBufferStagingBuffer.map();
	for(size_t i=0,e=size_t(numTriangles)*3*4; i<e; i+=4) {
		uint32_t* src1=&reinterpret_cast<uint32_t*>(packedAttribute1StagingBuffer.ptr)[i];
		uint32_t* src2=&reinterpret_cast<uint32_t*>(packedAttribute2StagingBuffer.ptr)[i];
		uint32_t* dest=&reinterpret_cast<uint32_t*>(singlePackedBufferStagingBuffer.ptr)[i*2];
		dest[0]=src1[0];  // posX
		dest[1]=src1[1];  // posY
		dest[2]=src1[2];  // posZ
		dest[3]=src2[3];  // packedColor
		dest[4]=src2[0];  // texCoord U
		dest[5]=src2[1];  // texCoord V
		dest[6]=src2[2];  // normalX+Y
		dest[7]=src1[3];  // normalZ+posW
	}

	// packedDAttributes
	packedDAttribute1StagingBuffer.map();
	packedDAttribute2StagingBuffer.map();
	packedDAttribute3StagingBuffer.map();
	for(size_t i=0,e=size_t(numTriangles)*3*2; i<e; i+=2) {
		reinterpret_cast<double*>(packedDAttribute1StagingBuffer.ptr)[i+0]=reinterpret_cast<float*>(packedAttribute1StagingBuffer.ptr)[i*2+0];
		reinterpret_cast<double*>(packedDAttribute1StagingBuffer.ptr)[i+1]=reinterpret_cast<float*>(packedAttribute1StagingBuffer.ptr)[i*2+1];
		reinterpret_cast<double*>(packedDAttribute2StagingBuffer.ptr)[i+0]=reinterpret_cast<float*>(packedAttribute1StagingBuffer.ptr)[i*2+2];
		uint32_t normalXY=reinterpret_cast<uint32_t*>(packedAttribute2StagingBuffer.ptr)[i*2+2];
		uint32_t normalZposW=reinterpret_cast<uint32_t*>(packedAttribute1StagingBuffer.ptr)[i*2+3];
		reinterpret_cast<uint64_t*>(packedDAttribute2StagingBuffer.ptr)[i+1]=normalXY+(uint64_t(normalZposW)<<32);
		uint32_t texU=reinterpret_cast<uint32_t*>(packedAttribute2StagingBuffer.ptr)[i*2+0];
		uint32_t texV=reinterpret_cast<uint32_t*>(packedAttribute2StagingBuffer.ptr)[i*2+1];
		reinterpret_cast<uint64_t*>(packedDAttribute3StagingBuffer.ptr)[i+0]=texU+(uint64_t(texV)<<32);
		reinterpret_cast<uint64_t*>(packedDAttribute3StagingBuffer.ptr)[i+1]=reinterpret_cast<uint32_t*>(packedAttribute2StagingBuffer.ptr)[i*2+3];
	}
	packedDAttribute1StagingBuffer.unmap();
	packedDAttribute2StagingBuffer.unmap();
	packedDAttribute3StagingBuffer.unmap();
	singlePackedBufferStagingBuffer.unmap();

	// sameVertexPackedAttrinutes
	sameVertexPackedAttribute1StagingBuffer.map();
	for(size_t i=0,e=size_t(numTriangles)*3*16; i<e; i+=16)
		memcpy(&reinterpret_cast<char*>(sameVertexPackedAttribute1StagingBuffer.ptr)[i],packedAttribute1StagingBuffer.ptr,16);
	sameVertexPackedAttribute1StagingBuffer.unmap();
	sameVertexPackedAttribute2StagingBuffer.map();
	for(size_t i=0,e=size_t(numTriangles)*3*16; i<e; i+=16)
		memcpy(&reinterpret_cast<char*>(sameVertexPackedAttribute2StagingBuffer.ptr)[i],packedAttribute2StagingBuffer.ptr,16);
	sameVertexPackedAttribute2StagingBuffer.unmap();
	packedAttribute1StagingBuffer.unmap();
	packedAttribute2StagingBuffer.unmap();

	// index
	indexStagingBuffer.map();
	for(uint32_t i=0,e=uint32_t(numTriangles)*3; i<e; i++)
		reinterpret_cast<uint32_t*>(indexStagingBuffer.ptr)[i]=i;
	indexStagingBuffer.unmap();

	// primitiveRestartIndex
	primitiveRestartIndexStagingBuffer.map();
	for(uint32_t i=0,j=0,e=uint32_t(numTriangles)*3; i<e;) {
		reinterpret_cast<uint32_t*>(primitiveRestartIndexStagingBuffer.ptr)[j++]=i++;
		reinterpret_cast<uint32_t*>(primitiveRestartIndexStagingBuffer.ptr)[j++]=i++;
		reinterpret_cast<uint32_t*>(primitiveRestartIndexStagingBuffer.ptr)[j++]=i++;
		reinterpret_cast<uint32_t*>(primitiveRestartIndexStagingBuffer.ptr)[j++]=-1;
	}
	primitiveRestartIndexStagingBuffer.unmap();

	// stripIndex
	stripIndexStagingBuffer.map();
	generateStripIndices(reinterpret_cast<uint32_t*>(stripIndexStagingBuffer.ptr), numTriangles/maxTriStripLength, maxTriStripLength);
	stripIndexStagingBuffer.unmap();

	// stripPrimitiveRestartIndex
	stripPrimitiveRestart3IndexStagingBuffer.map();
	generateStripPrimitiveRestartIndices(reinterpret_cast<uint32_t*>(stripPrimitiveRestart3IndexStagingBuffer.ptr), numTriangles/1, 1);
	stripPrimitiveRestart3IndexStagingBuffer.unmap();
	stripPrimitiveRestart4IndexStagingBuffer.map();
	generateStripPrimitiveRestartIndices(reinterpret_cast<uint32_t*>(stripPrimitiveRestart4IndexStagingBuffer.ptr), numTriangles/2, 2);
	stripPrimitiveRestart4IndexStagingBuffer.unmap();
	stripPrimitiveRestart7IndexStagingBuffer.map();
	generateStripPrimitiveRestartIndices(reinterpret_cast<uint32_t*>(stripPrimitiveRestart7IndexStagingBuffer.ptr), numTriangles/5, 5);
	stripPrimitiveRestart7IndexStagingBuffer.unmap();
	stripPrimitiveRestart10IndexStagingBuffer.map();
	generateStripPrimitiveRestartIndices(reinterpret_cast<uint32_t*>(stripPrimitiveRestart10IndexStagingBuffer.ptr), numTriangles/8, 8);
	stripPrimitiveRestart10IndexStagingBuffer.unmap();
	stripPrimitiveRestart1002IndexStagingBuffer.map();
	generateStripPrimitiveRestartIndices(reinterpret_cast<uint32_t*>(stripPrimitiveRestart1002IndexStagingBuffer.ptr), numTriangles/1000, 1000);
	stripPrimitiveRestart1002IndexStagingBuffer.unmap();
	primitiveRestartMinusOne2IndexStagingBuffer.map();
	generateStripPrimitiveRestartIndices(reinterpret_cast<uint32_t*>(primitiveRestartMinusOne2IndexStagingBuffer.ptr), numTriangles/1, 1, 2);
	primitiveRestartMinusOne2IndexStagingBuffer.unmap();
	primitiveRestartMinusOne5IndexStagingBuffer.map();
	generateStripPrimitiveRestartIndices(reinterpret_cast<uint32_t*>(primitiveRestartMinusOne5IndexStagingBuffer.ptr), numTriangles/1, 1, 5);
	primitiveRestartMinusOne5IndexStagingBuffer.unmap();
	minusOneIndexStagingBuffer.map();
	fill(reinterpret_cast<int32_t*>(minusOneIndexStagingBuffer.ptr),reinterpret_cast<int32_t*>(minusOneIndexStagingBuffer.ptr)+minusOneIndexBufferSize/4,int32_t(-1));
	minusOneIndexStagingBuffer.unmap();
	zeroIndexStagingBuffer.map();
	fill(reinterpret_cast<uint32_t*>(zeroIndexStagingBuffer.ptr),reinterpret_cast<uint32_t*>(zeroIndexStagingBuffer.ptr)+zeroIndexBufferSize/4,uint32_t(0));
	zeroIndexStagingBuffer.unmap();
	plusOneIndexStagingBuffer.map();
	fill(reinterpret_cast<uint32_t*>(plusOneIndexStagingBuffer.ptr),reinterpret_cast<uint32_t*>(plusOneIndexStagingBuffer.ptr)+plusOneIndexBufferSize/4,uint32_t(1));
	plusOneIndexStagingBuffer.unmap();

	// stripPackedAttributes
	generateStrips(
		reinterpret_cast<float*>(stripPackedAttribute1StagingBuffer.map()),numTriangles/maxTriStripLength,
		maxTriStripLength,triangleSize,framebufferExtent.width,framebufferExtent.height,true,
		2./framebufferExtent.width,2./framebufferExtent.height,-1.,-1.);
	for(size_t i=3,e=stripPackedDataBufferSize/4; i<e; i+=4)
		reinterpret_cast<uint32_t*>(stripPackedAttribute1StagingBuffer.ptr)[i]=0x3c003c00; // two half-floats, both set to one
	stripPackedAttribute1StagingBuffer.unmap();
	puint=reinterpret_cast<uint32_t*>(stripPackedAttribute2StagingBuffer.map());
	for(size_t i=0,e=stripPackedDataBufferSize/4; i<e; ) {
		puint[i++]=0x3f800000;  // texture U (float), one (1.f is 0x3f800000, 0.f is 0x00000000)
		puint[i++]=0x3f800000;  // texture V (float), one
		puint[i++]=0x3c003c00;  // normalX+Y (2x half), two times one (1.f in half-float is 0x3c00, 0.f is 0x0000)
		puint[i++]=0xffffffff;  // color
		puint[i++]=0x3f800000;  // texture U (float), one
		puint[i++]=0x3f800000;  // texture V (float), one
		puint[i++]=0x3c003c00;  // normalX+Y (2x half), two times one
		puint[i++]=0xffffffff;  // color
		puint[i++]=0x3f800000;  // texture U (float), one
		puint[i++]=0x3f800000;  // texture V (float), one
		puint[i++]=0x3c003c00;  // normalX+Y (2x half), two times one
		puint[i++]=0xffffffff;  // color
	}
	stripPackedAttribute2StagingBuffer.unmap();

	// sharedVertexPackedAttributes
	generateSharedVertexTriangles(
		reinterpret_cast<float*>(sharedVertexPackedAttribute1StagingBuffer.map()),numTriangles/maxTriStripLength,
		maxTriStripLength,triangleSize,framebufferExtent.width,framebufferExtent.height,true,
		2./framebufferExtent.width,2./framebufferExtent.height,-1.,-1.);
	for(size_t i=3,e=sharedVertexPackedDataBufferSize/4; i<e; i+=4)
		reinterpret_cast<uint32_t*>(sharedVertexPackedAttribute1StagingBuffer.ptr)[i]=0x3c003c00; // two half-floats, both set to one
	sharedVertexPackedAttribute1StagingBuffer.unmap();
	puint=reinterpret_cast<uint32_t*>(sharedVertexPackedAttribute2StagingBuffer.map());
	for(size_t i=0,e=sharedVertexPackedDataBufferSize/4; i<e; ) {
		puint[i++]=0x3f800000;  // texture U (float), one (1.f is 0x3f800000, 0.f is 0x00000000)
		puint[i++]=0x3f800000;  // texture V (float), one
		puint[i++]=0x3c003c00;  // normalX+Y (2x half), two times one (1.f in half-float is 0x3c00, 0.f is 0x0000)
		puint[i++]=0xffffffff;  // color
		puint[i++]=0x3f800000;  // texture U (float), one
		puint[i++]=0x3f800000;  // texture V (float), one
		puint[i++]=0x3c003c00;  // normalX+Y (2x half), two times one
		puint[i++]=0xffffffff;  // color
		puint[i++]=0x3f800000;  // texture U (float), one
		puint[i++]=0x3f800000;  // texture V (float), one
		puint[i++]=0x3c003c00;  // normalX+Y (2x half), two times one
		puint[i++]=0xffffffff;  // color
	}
	sharedVertexPackedAttribute2StagingBuffer.unmap();

	// copy data from staging to attribute and storage buffer
	submitNowCommandBuffer->copyBuffer(
		coordinate4StagingBuffer.buffer.get(),  // srcBuffer
		coordinate4Attribute.get(),             // dstBuffer
		1,                                      // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,coordinate4BufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		coordinate4StagingBuffer.buffer.get(),  // srcBuffer
		coordinate4Buffer.get(),                // dstBuffer
		1,                                      // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,coordinate4BufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		coordinate3StagingBuffer.buffer.get(),  // srcBuffer
		coordinate3Attribute.get(),             // dstBuffer
		1,                                      // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,coordinate3BufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		coordinate3StagingBuffer.buffer.get(),  // srcBuffer
		coordinate3Buffer.get(),                // dstBuffer
		1,                                      // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,coordinate3BufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		normalStagingBuffer.buffer.get(),  // srcBuffer
		normalAttribute.get(),             // dstBuffer
		1,                                     // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,normalBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		colorStagingBuffer.buffer.get(),  // srcBuffer
		colorAttribute.get(),             // dstBuffer
		1,                                     // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,colorBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		texCoordStagingBuffer.buffer.get(),  // srcBuffer
		texCoordAttribute.get(),             // dstBuffer
		1,                                     // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,texCoordBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		vec4Attribute1StagingBuffer.buffer.get(),  // srcBuffer
		vec4Attributes[0].get(),                   // dstBuffer
		1,                                         // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,coordinate4BufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		vec4Attribute1StagingBuffer.buffer.get(),  // srcBuffer
		vec4Buffers[0].get(),                      // dstBuffer
		1,                                         // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,coordinate4BufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		vec4Attribute1StagingBuffer.buffer.get(),  // srcBuffer
		vec3Buffers[0].get(),                      // dstBuffer
		1,                                         // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,coordinate3BufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		vec4Attribute1StagingBuffer.buffer.get(),  // srcBuffer
		vec4Attributes[1].get(),                   // dstBuffer
		1,                                         // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,coordinate4BufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		vec4Attribute1StagingBuffer.buffer.get(),  // srcBuffer
		vec4Buffers[1].get(),                      // dstBuffer
		1,                                         // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,coordinate4BufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		vec4Attribute1StagingBuffer.buffer.get(),  // srcBuffer
		vec3Buffers[1].get(),                      // dstBuffer
		1,                                         // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,coordinate3BufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		vec4Attribute2StagingBuffer.buffer.get(),  // srcBuffer
		vec4Attributes[2].get(),                   // dstBuffer
		1,                                         // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,coordinate4BufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		vec4Attribute2StagingBuffer.buffer.get(),  // srcBuffer
		vec4Buffers[2].get(),                      // dstBuffer
		1,                                         // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,coordinate4BufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		vec4Attribute2StagingBuffer.buffer.get(),  // srcBuffer
		vec3Buffers[2].get(),                      // dstBuffer
		1,                                         // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,coordinate3BufferSize)  // pRegions
	);
	for(size_t i=0; i<vec4u8Attributes.size(); i++)
		submitNowCommandBuffer->copyBuffer(
			vec4u8AttributeStagingBuffer.buffer.get(),  // srcBuffer
			vec4u8Attributes[i].get(),                  // dstBuffer
			1,                                          // regionCount
			&(const vk::BufferCopy&)vk::BufferCopy(0,0,vec4u8BufferSize)  // pRegions
		);
	submitNowCommandBuffer->copyBuffer(
		packedAttribute1StagingBuffer.buffer.get(),  // srcBuffer
		packedAttribute1.get(),                      // dstBuffer
		1,                                           // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,packedDataBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		packedAttribute2StagingBuffer.buffer.get(),  // srcBuffer
		packedAttribute2.get(),                      // dstBuffer
		1,                                           // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,packedDataBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		packedAttribute1StagingBuffer.buffer.get(),  // srcBuffer
		packedBuffer1.get(),                         // dstBuffer
		1,                                           // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,packedDataBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		packedAttribute2StagingBuffer.buffer.get(),  // srcBuffer
		packedBuffer2.get(),                         // dstBuffer
		1,                                           // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,packedDataBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		twoInterleavedBuffersStagingBuffer.buffer.get(),  // srcBuffer
		twoInterleavedAttributes.get(),                   // dstBuffer
		1,                                                // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,twoInterleavedBuffersSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		twoInterleavedBuffersStagingBuffer.buffer.get(),  // srcBuffer
		twoInterleavedBuffers.get(),                      // dstBuffer
		1,                                                // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,twoInterleavedBuffersSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		fourInterleavedBuffersStagingBuffer.buffer.get(),  // srcBuffer
		fourInterleavedAttributes.get(),                   // dstBuffer
		1,                                                 // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,fourInterleavedBuffersSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		fourInterleavedBuffersStagingBuffer.buffer.get(),  // srcBuffer
		fourInterleavedBuffers.get(),                      // dstBuffer
		1,                                                 // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,fourInterleavedBuffersSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		singlePackedBufferStagingBuffer.buffer.get(),  // srcBuffer
		singlePackedBuffer.get(),                      // dstBuffer
		1,                                             // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,packedDataBufferSize*2)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		packedDAttribute1StagingBuffer.buffer.get(),  // srcBuffer
		packedDAttribute1.get(),                      // dstBuffer
		1,                                            // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,packedDataBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		packedDAttribute2StagingBuffer.buffer.get(),  // srcBuffer
		packedDAttribute2.get(),                      // dstBuffer
		1,                                            // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,packedDataBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		packedDAttribute3StagingBuffer.buffer.get(),  // srcBuffer
		packedDAttribute3.get(),                      // dstBuffer
		1,                                            // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,packedDataBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		indexStagingBuffer.buffer.get(),  // srcBuffer
		indexBuffer.get(),                // dstBuffer
		1,                                // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,indexBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		primitiveRestartIndexStagingBuffer.buffer.get(),  // srcBuffer
		primitiveRestartIndexBuffer.get(),                // dstBuffer
		1,                                                // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,primitiveRestartIndexBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		stripIndexStagingBuffer.buffer.get(),  // srcBuffer
		stripIndexBuffer.get(),                // dstBuffer
		1,                                     // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,stripIndexBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		stripPrimitiveRestart3IndexStagingBuffer.buffer.get(),  // srcBuffer
		stripPrimitiveRestart3IndexBuffer.get(),                // dstBuffer
		1,                                                      // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,stripPrimitiveRestart3IndexBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		stripPrimitiveRestart4IndexStagingBuffer.buffer.get(),  // srcBuffer
		stripPrimitiveRestart4IndexBuffer.get(),                // dstBuffer
		1,                                                      // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,stripPrimitiveRestart4IndexBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		stripPrimitiveRestart7IndexStagingBuffer.buffer.get(),  // srcBuffer
		stripPrimitiveRestart7IndexBuffer.get(),                // dstBuffer
		1,                                                      // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,stripPrimitiveRestart7IndexBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		stripPrimitiveRestart10IndexStagingBuffer.buffer.get(),  // srcBuffer
		stripPrimitiveRestart10IndexBuffer.get(),                // dstBuffer
		1,                                                       // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,stripPrimitiveRestart10IndexBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		stripPrimitiveRestart1002IndexStagingBuffer.buffer.get(),  // srcBuffer
		stripPrimitiveRestart1002IndexBuffer.get(),                // dstBuffer
		1,                                                     // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,stripPrimitiveRestart1002IndexBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		primitiveRestartMinusOne2IndexStagingBuffer.buffer.get(),  // srcBuffer
		primitiveRestartMinusOne2IndexBuffer.get(),                // dstBuffer
		1,                                                         // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,primitiveRestartMinusOne2IndexBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		primitiveRestartMinusOne5IndexStagingBuffer.buffer.get(),  // srcBuffer
		primitiveRestartMinusOne5IndexBuffer.get(),                // dstBuffer
		1,                                                         // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,primitiveRestartMinusOne5IndexBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		minusOneIndexStagingBuffer.buffer.get(),  // srcBuffer
		minusOneIndexBuffer.get(),                // dstBuffer
		1,                                        // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,minusOneIndexBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		zeroIndexStagingBuffer.buffer.get(),  // srcBuffer
		zeroIndexBuffer.get(),                // dstBuffer
		1,                                    // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,zeroIndexBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		plusOneIndexStagingBuffer.buffer.get(),  // srcBuffer
		plusOneIndexBuffer.get(),                // dstBuffer
		1,                                       // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,plusOneIndexBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		stripPackedAttribute1StagingBuffer.buffer.get(),  // srcBuffer
		stripPackedAttribute1.get(),                      // dstBuffer
		1,                                                // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,stripPackedDataBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		stripPackedAttribute2StagingBuffer.buffer.get(),  // srcBuffer
		stripPackedAttribute2.get(),                      // dstBuffer
		1,                                                // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,stripPackedDataBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		sharedVertexPackedAttribute1StagingBuffer.buffer.get(),  // srcBuffer
		sharedVertexPackedAttribute1.get(),                      // dstBuffer
		1,                                                       // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,sharedVertexPackedDataBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		sharedVertexPackedAttribute2StagingBuffer.buffer.get(),  // srcBuffer
		sharedVertexPackedAttribute2.get(),                      // dstBuffer
		1,                                                       // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,sharedVertexPackedDataBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		sameVertexPackedAttribute1StagingBuffer.buffer.get(),  // srcBuffer
		sameVertexPackedAttribute1.get(),                      // dstBuffer
		1,                                                     // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,sameVertexPackedDataBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		sameVertexPackedAttribute2StagingBuffer.buffer.get(),  // srcBuffer
		sameVertexPackedAttribute2.get(),                      // dstBuffer
		1,                                                     // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,sameVertexPackedDataBufferSize)  // pRegions
	);

	// submit command buffer
	submitNowCommandBuffer->end();
	graphicsQueue.submit(
		vk::SubmitInfo(  // submits (vk::ArrayProxy)
			0,nullptr,nullptr,       // waitSemaphoreCount,pWaitSemaphores,pWaitDstStageMask
			1,&submitNowCommandBuffer.get(),  // commandBufferCount,pCommandBuffers
			0,nullptr                // signalSemaphoreCount,pSignalSemaphores
		),
		fence.get()  // fence
	);

	// wait for work to complete
	vk::Result r=device->waitForFences(
		fence.get(),   // fences (vk::ArrayProxy)
		VK_TRUE,       // waitAll
		uint64_t(3e9)  // timeout (3s)
	);
	if(r==vk::Result::eTimeout)
		throw std::runtime_error("GPU timeout. Task is probably hanging.");
	device->resetFences(
		fence.get()
	);

	// release memory (to avoid gpu out-of-memory error)
	coordinate4StagingBuffer.reset();
	coordinate3StagingBuffer.reset();
	normalStagingBuffer.reset();
	colorStagingBuffer.reset();
	texCoordStagingBuffer.reset();
	vec4Attribute1StagingBuffer.reset();
	vec4Attribute2StagingBuffer.reset();
	vec4u8AttributeStagingBuffer.reset();
	packedAttribute1StagingBuffer.reset();
	packedAttribute2StagingBuffer.reset();
	twoInterleavedBuffersStagingBuffer.reset();
	fourInterleavedBuffersStagingBuffer.reset();
	singlePackedBufferStagingBuffer.reset();
	packedDAttribute1StagingBuffer.reset();
	packedDAttribute2StagingBuffer.reset();
	packedDAttribute3StagingBuffer.reset();
	indexStagingBuffer.reset();
	primitiveRestartIndexStagingBuffer.reset();
	stripIndexStagingBuffer.reset();
	stripPrimitiveRestart3IndexStagingBuffer.reset();
	stripPrimitiveRestart4IndexStagingBuffer.reset();
	stripPrimitiveRestart7IndexStagingBuffer.reset();
	stripPrimitiveRestart10IndexStagingBuffer.reset();
	stripPrimitiveRestart1002IndexStagingBuffer.reset();
	primitiveRestartMinusOne2IndexStagingBuffer.reset();
	primitiveRestartMinusOne5IndexStagingBuffer.reset();
	minusOneIndexStagingBuffer.reset();
	stripPackedAttribute1StagingBuffer.reset();
	stripPackedAttribute2StagingBuffer.reset();
	sharedVertexPackedAttribute1StagingBuffer.reset();
	sharedVertexPackedAttribute2StagingBuffer.reset();
	sameVertexPackedAttribute1StagingBuffer.reset();
	sameVertexPackedAttribute2StagingBuffer.reset();

	// start new command buffer
	submitNowCommandBuffer->reset(vk::CommandBufferResetFlags());
	submitNowCommandBuffer->begin(
		vk::CommandBufferBeginInfo(
			vk::CommandBufferUsageFlagBits::eOneTimeSubmit,  // flags
			nullptr  // pInheritanceInfo
		)
	);

	// matrix attributes, buffers and uniforms
	size_t transformationMatrix4x4BufferSize=size_t(numTriangles)*16*sizeof(float); // 64MB
	size_t transformationMatrix4x3BufferSize=size_t(numTriangles)*12*sizeof(float);
	size_t transformationDMatrix4x4BufferSize=size_t(numTriangles)*16*sizeof(double); // 128MB
	size_t normalMatrix4x3BufferSize=size_t(numTriangles)*16*sizeof(float); // 64MB
	size_t transformationPATBufferSize=size_t(numTriangles)*8*sizeof(float);
	constexpr size_t viewAndProjectionMatricesBufferSize=(16+16+12)*sizeof(float);
	constexpr size_t viewAndProjectionDMatricesBufferSize=16*sizeof(double)+(16+12)*sizeof(float);
	constexpr size_t materialUniformBufferSize=4*12+4+4;
	constexpr size_t materialNotPackedUniformBufferSize=4*16+4+4;
	constexpr size_t globalLightUniformBufferSize=12;
	constexpr size_t lightUniformBufferSize=16+4*12;
	constexpr size_t lightNotPackedUniformBufferSize=5*16;
	constexpr size_t allInOneLightingUniformBufferSize=6*16;
	startTime=chrono::high_resolution_clock::now();
	createBuffer(singleMatrixUniformBuffer,    singleMatrixUniformMemory,          16*sizeof(float),                  false,vk::BufferUsageFlagBits::eUniformBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(singlePATBuffer,              singlePATMemory,                    8*sizeof(float),                   false,vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(sameMatrixAttribute,          sameMatrixAttributeMemory,          transformationMatrix4x4BufferSize, true, vk::BufferUsageFlagBits::eVertexBuffer |vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(sameMatrixBuffer,             sameMatrixBufferMemory,             transformationMatrix4x4BufferSize, true, vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(sameMatrixRowMajorBuffer,     sameMatrixRowMajorBufferMemory,     transformationMatrix4x4BufferSize, true, vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(sameMatrix4x3Buffer,          sameMatrix4x3BufferMemory,          transformationMatrix4x3BufferSize, true, vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(sameMatrix4x3RowMajorBuffer,  sameMatrix4x3RowMajorBufferMemory,  transformationMatrix4x3BufferSize, true, vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(sameDMatrixBuffer,            sameDMatrixBufferMemory,            transformationDMatrix4x4BufferSize,true, vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(samePATBuffer,                samePATBufferMemory,                transformationPATBufferSize,       true, vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(transformationMatrixAttribute,transformationMatrixAttributeMemory,transformationMatrix4x4BufferSize, true, vk::BufferUsageFlagBits::eVertexBuffer |vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(transformationMatrixBuffer,   transformationMatrixBufferMemory,   transformationMatrix4x4BufferSize, true, vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(normalMatrix4x3Buffer,        normalMatrix4x3Memory,              normalMatrix4x3BufferSize,         true, vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(viewAndProjectionMatricesUniformBuffer,viewAndProjectionMatricesMemory,viewAndProjectionMatricesBufferSize,false,vk::BufferUsageFlagBits::eUniformBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(viewAndProjectionDMatricesUniformBuffer,viewAndProjectionDMatricesMemory,viewAndProjectionDMatricesBufferSize,false,vk::BufferUsageFlagBits::eUniformBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(materialUniformBuffer,        materialUniformBufferMemory,        materialUniformBufferSize,         false,vk::BufferUsageFlagBits::eUniformBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(materialNotPackedUniformBuffer,materialNotPackedUniformBufferMemory,materialNotPackedUniformBufferSize,false,vk::BufferUsageFlagBits::eUniformBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(globalLightUniformBuffer,     globalLightUniformBufferMemory,     globalLightUniformBufferSize,      false,vk::BufferUsageFlagBits::eUniformBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(lightUniformBuffer,           lightUniformBufferMemory,           lightUniformBufferSize,            false,vk::BufferUsageFlagBits::eUniformBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(lightNotPackedUniformBuffer,  lightNotPackedUniformBufferMemory,  lightNotPackedUniformBufferSize,   false,vk::BufferUsageFlagBits::eUniformBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(allInOneLightingUniformBuffer,allInOneLightingUniformBufferMemory,allInOneLightingUniformBufferSize, false,vk::BufferUsageFlagBits::eUniformBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(indirectBuffer,               indirectBufferMemory,               (size_t(numTriangles)+1)*sizeof(vk::DrawIndirectCommand),                              true,vk::BufferUsageFlagBits::eIndirectBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(indirectStrideBuffer,         indirectStrideBufferMemory,         size_t(numTriangles)*max(sizeof(vk::DrawIndirectCommand),size_t(indirectRecordStride)),true,vk::BufferUsageFlagBits::eIndirectBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(indirectIndexedBuffer,        indirectIndexedBufferMemory,        (size_t(numTriangles)+1)*sizeof(vk::DrawIndexedIndirectCommand),                              true,vk::BufferUsageFlagBits::eIndirectBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	createBuffer(indirectIndexedStrideBuffer,  indirectIndexedStrideBufferMemory,  size_t(numTriangles)*max(sizeof(vk::DrawIndexedIndirectCommand),size_t(indirectRecordStride)),true,vk::BufferUsageFlagBits::eIndirectBuffer|vk::BufferUsageFlagBits::eTransferDst,bindInfoList);
	if(debug) {
		totalMeasurementTime=chrono::duration<double>(chrono::high_resolution_clock::now()-startTime).count();
		cout<<"Second buffer and memory set"<<endl;
		cout<<"   Creation time: "<<totalMeasurementTime*1000<<"ms"<<endl;
#if 0
		cout<<"Second buffer set memory requirements: "<<(transformationMatrix4x4BufferSize+
				transformationMatrix4x4BufferSize+transformationMatrix4x4BufferSize+transformationMatrix4x3BufferSize+
				transformationMatrix4x3BufferSize+transformationDMatrix4x4BufferSize+transformationMatrix4x4BufferSize+
				transformationMatrix4x4BufferSize+normalMatrix4x3BufferSize+transformationPATBufferSize+
				viewAndProjectionMatricesBufferSize+viewAndProjectionDMatricesBufferSize+materialUniformBufferSize+
				materialNotPackedUniformBufferSize+globalLightUniformBufferSize+lightUniformBufferSize+
				lightNotPackedUniformBufferSize+allInOneLightingUniformBufferSize)/1024/1024<<"MiB"<<endl;
#endif
	}

	if(sparseMode==SPARSE_NONE)
	{
		size_t totalMemorySize=0;
		auto startTime=chrono::high_resolution_clock::now();
		for(auto &bindInfo : bindInfoList) {
			device->bindBufferMemory(
				bindInfo.buffer,  // buffer
				bindInfo.memory,  // memory
				0  // memoryOffset
			);
			totalMemorySize+=bindInfo.size;
		}
		if(debug) {
			double totalMeasurementTime=chrono::duration<double>(chrono::high_resolution_clock::now()-startTime).count();
			cout<<"   Standard binding: "<<totalMeasurementTime*1000<<"ms"<<endl;
			cout<<"   (total amount of memory: "<<((totalMemorySize+512)/1024+512)/1024<<"MiB, number of memory allocations: "<<bindInfoList.size()<<")"<<endl;
		}
		bindInfoList.clear();
	}
	else
	{
		vector<vk::SparseBufferMemoryBindInfo> bufferBinds;
		vector<vk::SparseMemoryBind> memoryBinds;
		bufferBinds.reserve(bindInfoList.size());
		memoryBinds.reserve(bindInfoList.size());
		size_t numBlocks=0;
		size_t numMemObjs=0;
		for(auto &bindInfo : bindInfoList) {
			if(!bindInfo.sparseAllowed) {
				device->bindBufferMemory(
					bindInfo.buffer,  // buffer
					bindInfo.memory,  // memory
					0  // memoryOffset
				);
				continue;
			}
			auto& r=memoryBinds.emplace_back(
				0,  // resourceOffset
				bindInfo.size,  // size
				bindInfo.memory,  // memory
				0,  // memoryOffset
				vk::SparseMemoryBindFlags()  // flags
			);
			bufferBinds.emplace_back(
				bindInfo.buffer,  // buffer
				1,  // bindCount
				&r  // pBinds
			);
			numBlocks+=bindInfo.size/sparseBlockSize;
			numMemObjs++;
		}
		bindInfoList.clear();
		auto startTime=chrono::high_resolution_clock::now();
		sparseQueue.bindSparse(
			vk::BindSparseInfo(
				nullptr,  // waitSemaphores
				bufferBinds,  // bufferBinds
				nullptr,  // imageOpaqueBinds
				nullptr,  // imageBinds
				nullptr  // signalSemaphores
			),
			vk::Fence()
		);
		sparseQueue.waitIdle();
		if(debug) {
			double totalMeasurementTime=chrono::duration<double>(chrono::high_resolution_clock::now()-startTime).count();
			cout<<"   Sparse binding: "<<totalMeasurementTime*1000<<"ms\n"
			    <<"   Binding time of a memory block: "<<totalMeasurementTime/numBlocks*1e6<<"us"<<endl;
			cout<<"   (number of blocks: "<<numBlocks<<", number of memory objects: "<<numMemObjs<<")"<<endl;
		}
	}

	// single matrix uniform staging buffer
	const float singleMatrixData[]{
		1.f,0.f,0.f,0.f,
		0.f,1.f,0.f,0.f,
		0.f,0.f,1.f,0.f,
		2.f/framebufferExtent.width*64.f,0.f,0.f,1.f
	};
	StagingBuffer singleMatrixStagingBuffer(sizeof(singleMatrixData));
	memcpy(singleMatrixStagingBuffer.map(),singleMatrixData,sizeof(singleMatrixData));
	singleMatrixStagingBuffer.unmap();

	// single PAT uniform staging buffer
	const float singlePATData[]{
		0.f,0.f,0.f,1.f,  // zero rotation quaternion
		2.f/framebufferExtent.width*4.f,0.f,0.f,1.f,  // xyz translation + scale
	};
	StagingBuffer singlePATStagingBuffer(sizeof(singlePATData));
	memcpy(singlePATStagingBuffer.map(),singlePATData,sizeof(singlePATData));
	singlePATStagingBuffer.unmap();

	// same matrix staging buffers
	const float matrixData[16]{
		1.f,0.f,0.f,0.f,
		0.f,1.f,0.f,0.f,
		0.f,0.f,1.f,0.f,
		0.f,2.f/framebufferExtent.height*64.f,0.f,1.f,
	};
	const float matrixRowMajorData[16]{
		1.f,0.f,0.f,0.f,
		0.f,1.f,0.f,2.f/framebufferExtent.height*64.f,
		0.f,0.f,1.f,0.f,
		0.f,0.f,0.f,1.f,
	};
	const float matrix4x3Data[12]{
		1.f,0.f,0.f,
		0.f,1.f,0.f,
		0.f,0.f,1.f,
		0.f,2.f/framebufferExtent.height*64.f,0.f
	};
	const float matrix4x3RowMajorData[12]{
		1.f,0.f,0.f,0.f,
		0.f,1.f,0.f,2.f/framebufferExtent.height*64.f,
		0.f,0.f,1.f,0.f,
	};
	const double dmatrixData[16]{
		1.,0.,0.,0.,
		0.,1.,0.,0.,
		0.,0.,1.,0.,
		0.,2./framebufferExtent.height*64.,0.,1.
	};
	const float patData[8]{
		0.f,0.f,0.f,1.f,  // zero rotation quaternion
		0.f,2.f/framebufferExtent.height*64.f,0.f,1.f,  // xyz translation + scale
	};
	StagingBuffer sameMatrixStagingBuffer(transformationMatrix4x4BufferSize);
	StagingBuffer sameMatrixRowMajorStagingBuffer(transformationMatrix4x4BufferSize);
	StagingBuffer sameMatrix4x3StagingBuffer(transformationMatrix4x3BufferSize);
	StagingBuffer sameMatrix4x3RowMajorStagingBuffer(transformationMatrix4x3BufferSize);
	StagingBuffer sameDMatrixStagingBuffer(transformationDMatrix4x4BufferSize);
	StagingBuffer samePATStagingBuffer(transformationPATBufferSize);
	sameMatrixStagingBuffer.map();
	sameMatrixRowMajorStagingBuffer.map();
	sameMatrix4x3StagingBuffer.map();
	sameMatrix4x3RowMajorStagingBuffer.map();
	sameDMatrixStagingBuffer.map();
	samePATStagingBuffer.map();
	for(size_t i=0; i<size_t(numTriangles); i++)
		memcpy(reinterpret_cast<char*>(sameMatrixStagingBuffer.ptr)+(i*sizeof(matrixData)),matrixData,sizeof(matrixData));
	for(size_t i=0; i<size_t(numTriangles); i++)
		memcpy(reinterpret_cast<char*>(sameMatrixRowMajorStagingBuffer.ptr)+(i*sizeof(matrixRowMajorData)),matrixRowMajorData,sizeof(matrixRowMajorData));
	for(size_t i=0; i<size_t(numTriangles); i++)
		memcpy(reinterpret_cast<char*>(sameMatrix4x3StagingBuffer.ptr)+(i*sizeof(matrix4x3Data)),matrix4x3Data,sizeof(matrix4x3Data));
	for(size_t i=0; i<size_t(numTriangles); i++)
		memcpy(reinterpret_cast<char*>(sameMatrix4x3RowMajorStagingBuffer.ptr)+(i*sizeof(matrix4x3RowMajorData)),matrix4x3RowMajorData,sizeof(matrix4x3RowMajorData));
	for(size_t i=0; i<size_t(numTriangles); i++)
		memcpy(reinterpret_cast<char*>(sameDMatrixStagingBuffer.ptr)+(i*sizeof(dmatrixData)),dmatrixData,sizeof(dmatrixData));
	for(size_t i=0; i<size_t(numTriangles); i++)
		memcpy(reinterpret_cast<char*>(samePATStagingBuffer.ptr)+(i*sizeof(patData)),patData,sizeof(patData));
	sameMatrixStagingBuffer.unmap();
	sameMatrixRowMajorStagingBuffer.unmap();
	sameMatrix4x3StagingBuffer.unmap();
	sameMatrix4x3RowMajorStagingBuffer.unmap();
	sameDMatrixStagingBuffer.unmap();
	samePATStagingBuffer.unmap();

	// transformation matrix staging buffer
	StagingBuffer transformationMatrixStagingBuffer(transformationMatrix4x4BufferSize);
	generateMatrices(
		reinterpret_cast<float*>(transformationMatrixStagingBuffer.map()),numTriangles/2,triangleSize,
		framebufferExtent.width,framebufferExtent.height,
		2./framebufferExtent.width,2./framebufferExtent.height,0.,0.);
	transformationMatrixStagingBuffer.unmap();

	// normal matrix staging buffer
	const float normalMatrix4x3Data[]{
		1.f,0.f,0.f,0.f,
		0.f,1.f,0.f,0.f,
		0.f,0.f,1.f,0.f,
	};
	StagingBuffer normalMatrix4x3StagingBuffer(normalMatrix4x3BufferSize);
	normalMatrix4x3StagingBuffer.map();
	for(size_t i=0; i<size_t(numTriangles); i++)
		memcpy(reinterpret_cast<char*>(normalMatrix4x3StagingBuffer.ptr)+(i*sizeof(normalMatrix4x3Data)),normalMatrix4x3Data,sizeof(normalMatrix4x3Data));
	normalMatrix4x3StagingBuffer.unmap();

	// projection matrix staging buffers
	constexpr float viewAndProjectionMatricesData[]{
		1.f,0.f,0.f,0.f,
		0.f,1.f,0.f,0.f,
		0.f,0.f,1.f,0.f,
		0.f,0.f,0.f,1.f,

		1.f,0.f,0.f,0.f,
		0.f,1.f,0.f,0.f,
		0.f,0.f,1.f,0.f,
		0.f,0.f,0.f,1.f,

		1.f,0.f,0.f,0.f,
		0.f,1.f,0.f,0.f,
		0.f,0.f,1.f,0.f,
	};
	constexpr struct {
		double view[16] = {
			1.,0.,0.,0.,
			0.,1.,0.,0.,
			0.,0.,1.,0.,
			0.,0.,0.,1.,
		};
		float projection[16] = {
			1.,0.,0.,0.,
			0.,1.,0.,0.,
			0.,0.,1.,0.,
			0.,0.,0.,1.,
		};
		float normal[12] = {
			1.,0.,0.,0.,
			0.,1.,0.,0.,
			0.,0.,1.,0.,
		};
	} viewAndProjectionDMatricesData;
	static_assert(viewAndProjectionMatricesBufferSize==sizeof(viewAndProjectionMatricesData),"viewAndProjectionMatricesBufferSize must match size of viewAndProjectionMatricesData");
	static_assert(viewAndProjectionDMatricesBufferSize==sizeof(viewAndProjectionDMatricesData),"viewAndProjectionDMatricesBufferSize must match size of viewAndProjectionDMatricesData");
	StagingBuffer viewAndProjectionMatricesStagingBuffer(viewAndProjectionMatricesBufferSize);
	StagingBuffer viewAndProjectionDMatricesStagingBuffer(viewAndProjectionDMatricesBufferSize);
	memcpy(viewAndProjectionMatricesStagingBuffer.map(),viewAndProjectionMatricesData,viewAndProjectionMatricesBufferSize);
	memcpy(viewAndProjectionDMatricesStagingBuffer.map(),&viewAndProjectionDMatricesData,viewAndProjectionDMatricesBufferSize);
	viewAndProjectionMatricesStagingBuffer.unmap();
	viewAndProjectionDMatricesStagingBuffer.unmap();

	// material staging buffer
	constexpr struct {
		float f[13] = {
			0.8f,0.8f,0.8f,  // ambientColor
			0.8f,0.8f,0.8f,  // diffuseColor
			0.8f,0.8f,0.8f,  // specularColor
			0.0f,0.0f,0.0f,  // emissiveColor
			5.f,  // shininess
		};
		int32_t i[1] = {
			0x2100, // 0 - no texturing, 0x2100 - modulate, 0x1e01 - replace, 0x2101 - decal
		};
	} materialUniformData;
	static_assert(materialUniformBufferSize==sizeof(materialUniformData),"materialUniformBufferSize must match size of materialUniformData");
	StagingBuffer materialUniformStagingBuffer(materialUniformBufferSize);
	materialUniformStagingBuffer.map();
	memcpy(materialUniformStagingBuffer.ptr,&materialUniformData,sizeof(materialUniformData));
	materialUniformStagingBuffer.unmap();

	// material not packed staging buffer
	constexpr struct {
		float f[17] = {
			0.8f,0.8f,0.8f,1.f,  // ambientColor
			0.8f,0.8f,0.8f,1.f,  // diffuseColor
			0.8f,0.8f,0.8f,1.f,  // specularColor
			0.0f,0.0f,0.0f,1.f,  // emissiveColor
			5.f,  // shininess
		};
		int32_t i[1] = {
			0x2100, // 0 - no texturing, 0x2100 - modulate, 0x1e01 - replace, 0x2101 - decal
		};
	} materialNotPackedUniformData;
	static_assert(materialNotPackedUniformBufferSize==sizeof(materialNotPackedUniformData),"materialNotPackedUniformBufferSize must match size of materialNotPackedUniformData");
	StagingBuffer materialNotPackedUniformStagingBuffer(materialNotPackedUniformBufferSize);
	materialNotPackedUniformStagingBuffer.map();
	memcpy(materialNotPackedUniformStagingBuffer.ptr,&materialNotPackedUniformData,sizeof(materialNotPackedUniformData));
	materialNotPackedUniformStagingBuffer.unmap();

	// global light staging buffer
	const float globalLightUniformData[]{
		0.2f,0.2f,0.2f,  // globalAmbientLight
	};
	static_assert(globalLightUniformBufferSize==sizeof(globalLightUniformData),"globalLightUniformBufferSize must match size of globalLightUniformData");
	StagingBuffer globalLightUniformStagingBuffer(globalLightUniformBufferSize);
	globalLightUniformStagingBuffer.map();
	memcpy(globalLightUniformStagingBuffer.ptr,globalLightUniformData,sizeof(globalLightUniformData));
	globalLightUniformStagingBuffer.unmap();

	// light source staging buffer
	const float lightUniformData[]{
		-0.4f,0.4f,0.2f,1.0f,  // lightPosition
		1.0f,0.0f,0.0f,  // lightAttenuation
		0.2f,0.2f,0.2f,  // ambientLight
		0.6f,0.6f,0.6f,  // diffuseLight
		0.6f,0.6f,0.6f,  // specularLight
	};
	static_assert(lightUniformBufferSize==sizeof(lightUniformData),"lightUniformBufferSize must match size of lightUniformData");
	StagingBuffer lightUniformStagingBuffer(lightUniformBufferSize);
	lightUniformStagingBuffer.map();
	memcpy(lightUniformStagingBuffer.ptr,lightUniformData,sizeof(lightUniformData));
	lightUniformStagingBuffer.unmap();

	// light source staging buffer
	const float lightNotPackedUniformData[]{
		-0.4f,0.4f,0.2f,1.f,  // lightPosition
		1.0f,0.0f,0.0f,0.f,  // lightAttenuation
		0.2f,0.2f,0.2f,1.f,  // ambientLight
		0.6f,0.6f,0.6f,1.f,  // diffuseLight
		0.6f,0.6f,0.6f,1.f,  // specularLight
	};
	static_assert(lightNotPackedUniformBufferSize==sizeof(lightNotPackedUniformData),"lightNotPackedUniformBufferSize must match size of lightNotPackedUniformData");
	StagingBuffer lightNotPackedUniformStagingBuffer(lightNotPackedUniformBufferSize);
	lightNotPackedUniformStagingBuffer.map();
	memcpy(lightNotPackedUniformStagingBuffer.ptr,lightNotPackedUniformData,sizeof(lightNotPackedUniformData));
	lightNotPackedUniformStagingBuffer.unmap();

	// all-in-one lighting staging buffer
	const float allInOneLightingUniformData[]{
		0.8f,0.8f,0.8f,1.f,  // ambientColor
		0.8f,0.8f,0.8f,1.f,  // diffuseColor
		0.2f,0.2f,0.2f,0.f,  // globalAmbientLight
		-0.4f,0.4f,0.2f,  // lightPosition
		1.0f,0.0f,0.0f,  // lightAttenuation
		0.2f,0.2f,0.2f,  // ambientLight
		0.6f,0.6f,0.6f,  // diffuseLight
	};
	static_assert(allInOneLightingUniformBufferSize==sizeof(allInOneLightingUniformData),"allInOneLightingUniformBufferSize must match size of allInOneLightingUniformData");
	StagingBuffer allInOneLightingUniformStagingBuffer(allInOneLightingUniformBufferSize);
	allInOneLightingUniformStagingBuffer.map();
	memcpy(allInOneLightingUniformStagingBuffer.ptr,allInOneLightingUniformData,sizeof(allInOneLightingUniformData));
	allInOneLightingUniformStagingBuffer.unmap();

	// copy data from staging to attributes, buffers and uniforms
	submitNowCommandBuffer->copyBuffer(
		singleMatrixStagingBuffer.buffer.get(),  // srcBuffer
		singleMatrixUniformBuffer.get(),         // dstBuffer
		1,                                       // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,sizeof(singleMatrixData))  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		singlePATStagingBuffer.buffer.get(),  // srcBuffer
		singlePATBuffer.get(),                // dstBuffer
		1,                                    // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,sizeof(singlePATBuffer))  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		sameMatrixStagingBuffer.buffer.get(),  // srcBuffer
		sameMatrixAttribute.get(),             // dstBuffer
		1,                                     // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,transformationMatrix4x4BufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		sameMatrixStagingBuffer.buffer.get(),  // srcBuffer
		sameMatrixBuffer.get(),                // dstBuffer
		1,                                     // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,transformationMatrix4x4BufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		sameMatrixRowMajorStagingBuffer.buffer.get(),  // srcBuffer
		sameMatrixRowMajorBuffer.get(),                // dstBuffer
		1,                                             // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,transformationMatrix4x4BufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		sameMatrix4x3StagingBuffer.buffer.get(),  // srcBuffer
		sameMatrix4x3Buffer.get(),                // dstBuffer
		1,                                        // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,transformationMatrix4x3BufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		sameMatrix4x3RowMajorStagingBuffer.buffer.get(),  // srcBuffer
		sameMatrix4x3RowMajorBuffer.get(),                // dstBuffer
		1,                                                // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,transformationMatrix4x3BufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		sameDMatrixStagingBuffer.buffer.get(),  // srcBuffer
		sameDMatrixBuffer.get(),                // dstBuffer
		1,                                     // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,transformationDMatrix4x4BufferSize)  // pRegions
	);
	assert(sameDMatrixStagingBuffer.ptr==nullptr && "Staging buffer must be unmapped here.");
	::sameDMatrixStagingBuffer=move(sameDMatrixStagingBuffer.buffer);
	sameDMatrixStagingBufferMemory=move(sameDMatrixStagingBuffer.memory);
	sameDMatrixStagingBufferSize=transformationDMatrix4x4BufferSize;
	submitNowCommandBuffer->copyBuffer(
		samePATStagingBuffer.buffer.get(),  // srcBuffer
		samePATBuffer.get(),                // dstBuffer
		1,                                  // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,transformationPATBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		transformationMatrixStagingBuffer.buffer.get(),  // srcBuffer
		transformationMatrixAttribute.get(),             // dstBuffer
		1,                                               // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,transformationMatrix4x4BufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		transformationMatrixStagingBuffer.buffer.get(),  // srcBuffer
		transformationMatrixBuffer.get(),                // dstBuffer
		1,                                               // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,transformationMatrix4x4BufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		normalMatrix4x3StagingBuffer.buffer.get(),  // srcBuffer
		normalMatrix4x3Buffer.get(),                // dstBuffer
		1,                                          // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,normalMatrix4x3BufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		viewAndProjectionMatricesStagingBuffer.buffer.get(),  // srcBuffer
		viewAndProjectionMatricesUniformBuffer.get(),         // dstBuffer
		1,                                                    // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,viewAndProjectionMatricesBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		viewAndProjectionDMatricesStagingBuffer.buffer.get(),  // srcBuffer
		viewAndProjectionDMatricesUniformBuffer.get(),         // dstBuffer
		1,                                                     // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,viewAndProjectionDMatricesBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		materialUniformStagingBuffer.buffer.get(),  // srcBuffer
		materialUniformBuffer.get(),                // dstBuffer
		1,                                          // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,materialUniformBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		materialNotPackedUniformStagingBuffer.buffer.get(),  // srcBuffer
		materialNotPackedUniformBuffer.get(),                // dstBuffer
		1,                                                   // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,materialNotPackedUniformBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		globalLightUniformStagingBuffer.buffer.get(),  // srcBuffer
		globalLightUniformBuffer.get(),                // dstBuffer
		1,                                             // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,globalLightUniformBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		lightUniformStagingBuffer.buffer.get(),  // srcBuffer
		lightUniformBuffer.get(),                // dstBuffer
		1,                                       // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,lightUniformBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		lightNotPackedUniformStagingBuffer.buffer.get(),  // srcBuffer
		lightNotPackedUniformBuffer.get(),                // dstBuffer
		1,                                                // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,lightNotPackedUniformBufferSize)  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		allInOneLightingUniformStagingBuffer.buffer.get(),  // srcBuffer
		allInOneLightingUniformBuffer.get(),                // dstBuffer
		1,                                                  // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,allInOneLightingUniformBufferSize)  // pRegions
	);

	// indirect staging buffer
	StagingBuffer indirectStagingBuffer((size_t(numTriangles)+1)*sizeof(vk::DrawIndirectCommand));
	auto indirectBufferPtr=reinterpret_cast<vk::DrawIndirectCommand*>(indirectStagingBuffer.map());
	for(size_t i=0; i<size_t(numTriangles); i++) {
		indirectBufferPtr[i].vertexCount=3;
		indirectBufferPtr[i].instanceCount=1;
		indirectBufferPtr[i].firstVertex=uint32_t(i)*3;
		indirectBufferPtr[i].firstInstance=0;
	}
	indirectBufferPtr[numTriangles].vertexCount=3;
	indirectBufferPtr[numTriangles].instanceCount=numTriangles;
	indirectBufferPtr[numTriangles].firstVertex=0;
	indirectBufferPtr[numTriangles].firstInstance=0;
	indirectStagingBuffer.unmap();

	// indirect stride staging buffer
	union DrawIndirectStrideCommand {
		vk::DrawIndirectCommand c;
		array<char,indirectRecordStride> s;
	};
	StagingBuffer indirectStrideStagingBuffer(size_t(numTriangles)*sizeof(DrawIndirectStrideCommand));
	auto indirectStrideBufferPtr=reinterpret_cast<DrawIndirectStrideCommand*>(indirectStrideStagingBuffer.map());
	memset(indirectStrideBufferPtr, size_t(numTriangles)*sizeof(DrawIndirectStrideCommand), 0);
	for(size_t i=0; i<size_t(numTriangles); i++) {
		indirectStrideBufferPtr[i].c.vertexCount=3;
		indirectStrideBufferPtr[i].c.instanceCount=1;
		indirectStrideBufferPtr[i].c.firstVertex=uint32_t(i)*3;
		indirectStrideBufferPtr[i].c.firstInstance=0;
	}
	indirectStrideStagingBuffer.unmap();

	// indirect indexed staging buffer
	StagingBuffer indirectIndexedStagingBuffer((size_t(numTriangles)+1)*sizeof(vk::DrawIndexedIndirectCommand));
	auto indirectIndexedBufferPtr=reinterpret_cast<vk::DrawIndexedIndirectCommand*>(indirectIndexedStagingBuffer.map());
	for(size_t i=0; i<size_t(numTriangles); i++) {
		indirectIndexedBufferPtr[i].indexCount=3;
		indirectIndexedBufferPtr[i].instanceCount=1;
		indirectIndexedBufferPtr[i].firstIndex=uint32_t(i)*3;
		indirectIndexedBufferPtr[i].vertexOffset=0;
		indirectIndexedBufferPtr[i].firstInstance=0;
	}
	indirectIndexedBufferPtr[numTriangles].indexCount=3;
	indirectIndexedBufferPtr[numTriangles].instanceCount=numTriangles;
	indirectIndexedBufferPtr[numTriangles].firstIndex=0;
	indirectIndexedBufferPtr[numTriangles].vertexOffset=0;
	indirectIndexedBufferPtr[numTriangles].firstInstance=0;
	indirectIndexedStagingBuffer.unmap();

	// indirect indexed stride staging buffer
	union DrawIndexedIndirectStrideCommand {
		vk::DrawIndexedIndirectCommand c;
		array<char,indirectRecordStride> s;
	};
	StagingBuffer indirectIndexedStrideStagingBuffer(size_t(numTriangles)*sizeof(DrawIndexedIndirectStrideCommand));
	auto indirectIndexedStrideBufferPtr=reinterpret_cast<DrawIndexedIndirectStrideCommand*>(indirectIndexedStrideStagingBuffer.map());
	memset(indirectIndexedStrideBufferPtr, size_t(numTriangles)*sizeof(DrawIndexedIndirectStrideCommand), 0);
	for(size_t i=0; i<size_t(numTriangles); i++) {
		indirectIndexedStrideBufferPtr[i].c.indexCount=3;
		indirectIndexedStrideBufferPtr[i].c.instanceCount=1;
		indirectIndexedStrideBufferPtr[i].c.firstIndex=uint32_t(i)*3;
		indirectIndexedStrideBufferPtr[i].c.vertexOffset=0;
		indirectIndexedStrideBufferPtr[i].c.firstInstance=0;
	}
	indirectIndexedStrideStagingBuffer.unmap();

	// copy data from staging to uniform buffer
	submitNowCommandBuffer->copyBuffer(
		indirectStagingBuffer.buffer.get(),  // srcBuffer
		indirectBuffer.get(),                // dstBuffer
		1,                                   // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,(size_t(numTriangles)+1)*sizeof(vk::DrawIndirectCommand))  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		indirectStrideStagingBuffer.buffer.get(),  // srcBuffer
		indirectStrideBuffer.get(),                // dstBuffer
		1,                                         // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,size_t(numTriangles)*sizeof(DrawIndirectStrideCommand))  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		indirectIndexedStagingBuffer.buffer.get(),  // srcBuffer
		indirectIndexedBuffer.get(),                // dstBuffer
		1,                                          // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,(size_t(numTriangles)+1)*sizeof(vk::DrawIndexedIndirectCommand))  // pRegions
	);
	submitNowCommandBuffer->copyBuffer(
		indirectIndexedStrideStagingBuffer.buffer.get(),  // srcBuffer
		indirectIndexedStrideBuffer.get(),                // dstBuffer
		1,                                                // regionCount
		&(const vk::BufferCopy&)vk::BufferCopy(0,0,size_t(numTriangles)*sizeof(DrawIndexedIndirectStrideCommand))  // pRegions
	);

	// single texel image
	StagingBuffer singleTexelStagingBuffer(4);
	singleTexelStagingBuffer.map();
	reinterpret_cast<uint32_t*>(singleTexelStagingBuffer.ptr)[0]=0xffffffff;
	singleTexelStagingBuffer.unmap();
	submitNowCommandBuffer->pipelineBarrier(
		vk::PipelineStageFlagBits::eTopOfPipe,  // srcStageMask
		vk::PipelineStageFlagBits::eTransfer,  // dstStageMask
		vk::DependencyFlags(),  // dependencyFlags
		nullptr,  // memoryBarriers
		nullptr,  // bufferMemoryBarriers
		vk::ImageMemoryBarrier(  // imageMemoryBarriers
			vk::AccessFlags(),  // srcAccessMask
			vk::AccessFlagBits::eTransferWrite,  // dstAccessMask
			vk::ImageLayout::eUndefined,  // oldLayout
			vk::ImageLayout::eTransferDstOptimal,  // newLayout
			VK_QUEUE_FAMILY_IGNORED,  // srcQueueFamilyIndex
			VK_QUEUE_FAMILY_IGNORED,  // dstQueueFamilyIndex
			singleTexelImage.get(),  // image
			vk::ImageSubresourceRange(  // subresourceRange
				vk::ImageAspectFlagBits::eColor,  // aspectMask
				0,  // baseMipLevel
				1,  // levelCount
				0,  // baseArrayLayer
				1   // layerCount
			)
		)
	);
	submitNowCommandBuffer->copyBufferToImage(
		singleTexelStagingBuffer.buffer.get(),  // srcBuffer
		singleTexelImage.get(),                 // dstImage
		vk::ImageLayout::eTransferDstOptimal,   // dstImageLayout
		vk::BufferImageCopy(  // regions
			0,  // bufferOffset
			0,  // bufferRowLength
			0,  // bufferImageHeight
			vk::ImageSubresourceLayers(  // imageSubresource
				vk::ImageAspectFlagBits::eColor,  // aspectMask
				0,  // mipLevel
				0,  // baseArrayLayer
				1   // layerCount
			),
			vk::Offset3D(0,0,0),  // imageOffset
			vk::Extent3D(1,1,1)   // imageExtent
		)
	);
	submitNowCommandBuffer->pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer,  // srcStageMask
		vk::PipelineStageFlagBits::eFragmentShader,  // dstStageMask
		vk::DependencyFlags(),  // dependencyFlags
		nullptr,  // memoryBarriers
		nullptr,  // bufferMemoryBarriers
		vk::ImageMemoryBarrier(  // imageMemoryBarriers
			vk::AccessFlagBits::eTransferWrite,  // srcAccessMask
			vk::AccessFlagBits::eShaderRead,  // dstAccessMask
			vk::ImageLayout::eTransferDstOptimal,  // oldLayout
			vk::ImageLayout::eShaderReadOnlyOptimal,  // newLayout
			VK_QUEUE_FAMILY_IGNORED,  // srcQueueFamilyIndex
			VK_QUEUE_FAMILY_IGNORED,  // dstQueueFamilyIndex
			singleTexelImage.get(),  // image
			vk::ImageSubresourceRange(  // subresourceRange
				vk::ImageAspectFlagBits::eColor,  // aspectMask
				0,  // baseMipLevel
				1,  // levelCount
				0,  // baseArrayLayer
				1   // layerCount
			)
		)
	);

	// submit command buffer
	submitNowCommandBuffer->end();
	graphicsQueue.submit(
		vk::SubmitInfo(  // submits (vk::ArrayProxy)
			0,nullptr,nullptr,       // waitSemaphoreCount,pWaitSemaphores,pWaitDstStageMask
			1,&submitNowCommandBuffer.get(),  // commandBufferCount,pCommandBuffers
			0,nullptr                // signalSemaphoreCount,pSignalSemaphores
		),
		fence.get()  // fence
	);

	// wait for work to complete
	r=device->waitForFences(
		fence.get(),   // fences (vk::ArrayProxy)
		VK_TRUE,       // waitAll
		uint64_t(3e9)  // timeout (3s)
	);
	if(r==vk::Result::eTimeout)
		throw std::runtime_error("GPU timeout. Task is probably hanging.");
	device->resetFences(
		fence.get()
	);


	// descriptor sets
	descriptorPool=
		device->createDescriptorPoolUnique(
			vk::DescriptorPoolCreateInfo(
				vk::DescriptorPoolCreateFlags(),  // flags
				33,  // maxSets
				3,  // poolSizeCount
				array<vk::DescriptorPoolSize,3>{  // pPoolSizes
					vk::DescriptorPoolSize(
						vk::DescriptorType::eUniformBuffer,  // type
						24  // descriptorCount
					),
					vk::DescriptorPoolSize(
						vk::DescriptorType::eStorageBuffer,  // type
						45  // descriptorCount
					),
					vk::DescriptorPoolSize(
						vk::DescriptorType::eCombinedImageSampler,  // type
						2  // descriptorCount
					),
				}.data()
			)
		);
	oneUniformVSDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&oneUniformVSDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	one4fUniformFSDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&oneUniformFSDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	one4bUniformFSDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&oneUniformFSDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	coordinate4BufferDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&oneBufferDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	coordinate3BufferDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&oneBufferDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	sameMatrixBufferDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&oneBufferDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	transformationMatrixBufferDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&oneBufferDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	singlePackedBufferDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&oneBufferDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	twoBuffersDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&twoBuffersDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	twoInterleavedBuffersDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&oneBufferDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	twoPackedBuffersDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&twoBuffersDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	twoBuffer3DescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&twoBuffersDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	threeBuffersDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&threeBuffersDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	fourBuffersDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&fourBuffersDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	fourBuffer3DescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&fourBuffersDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	fourInterleavedBuffersDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&oneBufferDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	threeBuffersInGSDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&threeBuffersInGSDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	threeUniformFSDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&threeUniformFSDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	transformationThreeMatricesDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&bufferAndUniformDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	transformationThreeMatricesRowMajorDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&bufferAndUniformDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	transformationThreeMatrices4x3DescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&bufferAndUniformDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	transformationThreeMatrices4x3RowMajorDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&bufferAndUniformDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	transformationThreeDMatricesDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&bufferAndUniformDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	transformationTwoMatricesDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&twoBuffersDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	transformationTwoMatricesAndPATDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&bufferAndUniformDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	transformationTwoMatricesAndSinglePATDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&bufferAndUniformDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	transformationFiveMatricesDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&twoBuffersAndUniformDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	transformationFiveMatricesUsingGSDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&fourBuffersAndUniformInGSDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	transformationFiveMatricesUsingGSAndAttributesDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&twoBuffersAndUniformInGSDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	phongTexturedThreeDMatricesUsingGSAndAttributesDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&bufferAndUniformInGSDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	phongTexturedDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&phongTexturedDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	phongTexturedNotPackedDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&phongTexturedDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	allInOneLightingUniformDescriptorSet=
		device->allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo(
				descriptorPool.get(),  // descriptorPool
				1,  // descriptorSetCount
				&oneUniformFSDescriptorSetLayout.get()  // pSetLayouts
			)
		)[0];
	device->updateDescriptorSets(
		array<vk::WriteDescriptorSet,46>{{  // descriptorWrites
			{
				oneUniformVSDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eUniformBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						singleMatrixUniformBuffer.get(),  // buffer
						0,  // offset
						16*sizeof(float)  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				one4fUniformFSDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eUniformBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						materialUniformBuffer.get(),  // buffer
						0,  // offset
						4*sizeof(float)  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				one4bUniformFSDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eUniformBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						materialUniformBuffer.get(),  // buffer
						0*sizeof(float),  // offset
						4  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				coordinate4BufferDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						coordinate4Buffer.get(),  // buffer
						0,  // offset
						coordinate4BufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				coordinate3BufferDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						coordinate3Buffer.get(),  // buffer
						0,  // offset
						coordinate3BufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				sameMatrixBufferDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						sameMatrixBuffer.get(),  // buffer
						0,  // offset
						size_t(numTriangles)*16*sizeof(float)  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				transformationMatrixBufferDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						transformationMatrixBuffer.get(),  // buffer
						0,  // offset
						size_t(numTriangles)*16*sizeof(float)  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				singlePackedBufferDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						singlePackedBuffer.get(),  // buffer
						0,  // offset
						size_t(numTriangles)*3*32  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				twoBuffersDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				2,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,2>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						coordinate4Buffer.get(),  // buffer
						0,  // offset
						coordinate4BufferSize  // range
					),
					vk::DescriptorBufferInfo(
						vec4Buffers[0].get(),  // buffer
						0,  // offset
						coordinate4BufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				twoInterleavedBuffersDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						twoInterleavedBuffers.get(),  // buffer
						0,  // offset
						twoInterleavedBuffersSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				twoPackedBuffersDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				2,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,2>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						packedBuffer1.get(),  // buffer
						0,  // offset
						packedDataBufferSize  // range
					),
					vk::DescriptorBufferInfo(
						packedBuffer2.get(),  // buffer
						0,  // offset
						packedDataBufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				twoBuffer3DescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				2,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,2>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						coordinate3Buffer.get(),  // buffer
						0,  // offset
						coordinate3BufferSize  // range
					),
					vk::DescriptorBufferInfo(
						vec3Buffers[0].get(),  // buffer
						0,  // offset
						coordinate3BufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				threeBuffersDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				3,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,3>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						packedBuffer1.get(),  // buffer
						0,  // offset
						packedDataBufferSize  // range
					),
					vk::DescriptorBufferInfo(
						packedBuffer2.get(),  // buffer
						0,  // offset
						packedDataBufferSize  // range
					),
					vk::DescriptorBufferInfo(
						sameMatrixBuffer.get(),  // buffer
						0,  // offset
						size_t(numTriangles)*16*sizeof(float)  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				fourBuffersDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				4,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,4>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						coordinate4Buffer.get(),  // buffer
						0,  // offset
						coordinate4BufferSize  // range
					),
					vk::DescriptorBufferInfo(
						vec4Buffers[0].get(),  // buffer
						0,  // offset
						coordinate4BufferSize  // range
					),
					vk::DescriptorBufferInfo(
						vec4Buffers[1].get(),  // buffer
						0,  // offset
						coordinate4BufferSize  // range
					),
					vk::DescriptorBufferInfo(
						vec4Buffers[2].get(),  // buffer
						0,  // offset
						coordinate4BufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				fourBuffer3DescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				4,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,4>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						coordinate3Buffer.get(),  // buffer
						0,  // offset
						coordinate3BufferSize  // range
					),
					vk::DescriptorBufferInfo(
						vec3Buffers[0].get(),  // buffer
						0,  // offset
						coordinate3BufferSize  // range
					),
					vk::DescriptorBufferInfo(
						vec3Buffers[1].get(),  // buffer
						0,  // offset
						coordinate3BufferSize  // range
					),
					vk::DescriptorBufferInfo(
						vec3Buffers[2].get(),  // buffer
						0,  // offset
						coordinate3BufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				fourInterleavedBuffersDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						fourInterleavedBuffers.get(),  // buffer
						0,  // offset
						fourInterleavedBuffersSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				threeBuffersInGSDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				3,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,3>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						packedBuffer1.get(),  // buffer
						0,  // offset
						packedDataBufferSize  // range
					),
					vk::DescriptorBufferInfo(
						packedBuffer2.get(),  // buffer
						0,  // offset
						packedDataBufferSize  // range
					),
					vk::DescriptorBufferInfo(
						sameMatrixBuffer.get(),  // buffer
						0,  // offset
						size_t(numTriangles)*16*sizeof(float)  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				transformationThreeMatricesDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						sameMatrixBuffer.get(),  // buffer
						0,  // offset
						transformationMatrix4x4BufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				transformationThreeMatricesRowMajorDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						sameMatrixRowMajorBuffer.get(),  // buffer
						0,  // offset
						transformationMatrix4x4BufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				transformationThreeMatrices4x3DescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						sameMatrix4x3Buffer.get(),  // buffer
						0,  // offset
						transformationMatrix4x3BufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				transformationThreeMatrices4x3RowMajorDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						sameMatrix4x3RowMajorBuffer.get(),  // buffer
						0,  // offset
						transformationMatrix4x3BufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				transformationThreeDMatricesDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						sameDMatrixBuffer.get(),  // buffer
						0,  // offset
						transformationDMatrix4x4BufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				transformationTwoMatricesDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				2,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,2>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						sameMatrixBuffer.get(),  // buffer
						0,  // offset
						transformationMatrix4x4BufferSize  // range
					),
					vk::DescriptorBufferInfo(
						normalMatrix4x3Buffer.get(),  // buffer
						0,  // offset
						normalMatrix4x3BufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				transformationTwoMatricesAndPATDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						samePATBuffer.get(),  // buffer
						0,  // offset
						transformationPATBufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				transformationTwoMatricesAndSinglePATDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						singlePATBuffer.get(),  // buffer
						0,  // offset
						8*sizeof(float)  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				transformationThreeMatricesDescriptorSet,  // dstSet
				1,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eUniformBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						viewAndProjectionMatricesUniformBuffer.get(),  // buffer
						0,  // offset
						viewAndProjectionMatricesBufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				transformationThreeMatricesRowMajorDescriptorSet,  // dstSet
				1,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eUniformBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						viewAndProjectionMatricesUniformBuffer.get(),  // buffer
						0,  // offset
						viewAndProjectionMatricesBufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				transformationThreeMatrices4x3DescriptorSet,  // dstSet
				1,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eUniformBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						viewAndProjectionMatricesUniformBuffer.get(),  // buffer
						0,  // offset
						viewAndProjectionMatricesBufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				transformationThreeMatrices4x3RowMajorDescriptorSet,  // dstSet
				1,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eUniformBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						viewAndProjectionMatricesUniformBuffer.get(),  // buffer
						0,  // offset
						viewAndProjectionMatricesBufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				transformationThreeDMatricesDescriptorSet,  // dstSet
				1,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eUniformBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						viewAndProjectionDMatricesUniformBuffer.get(),  // buffer
						0,  // offset
						viewAndProjectionDMatricesBufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				transformationTwoMatricesAndPATDescriptorSet,  // dstSet
				1,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eUniformBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						viewAndProjectionMatricesUniformBuffer.get(),  // buffer
						0,  // offset
						viewAndProjectionMatricesBufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				transformationTwoMatricesAndSinglePATDescriptorSet,  // dstSet
				1,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eUniformBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						viewAndProjectionMatricesUniformBuffer.get(),  // buffer
						0,  // offset
						viewAndProjectionMatricesBufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				transformationFiveMatricesDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				2,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,2>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						sameMatrixBuffer.get(),  // buffer
						0,  // offset
						transformationMatrix4x4BufferSize  // range
					),
					vk::DescriptorBufferInfo(
						normalMatrix4x3Buffer.get(),  // buffer
						0,  // offset
						normalMatrix4x3BufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				transformationFiveMatricesDescriptorSet,  // dstSet
				2,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eUniformBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						viewAndProjectionMatricesUniformBuffer.get(),  // buffer
						0,  // offset
						viewAndProjectionMatricesBufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				transformationFiveMatricesUsingGSDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				4,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,4>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						packedBuffer1.get(),  // buffer
						0,  // offset
						packedDataBufferSize  // range
					),
					vk::DescriptorBufferInfo(
						packedBuffer2.get(),  // buffer
						0,  // offset
						packedDataBufferSize  // range
					),
					vk::DescriptorBufferInfo(
						sameMatrixBuffer.get(),  // buffer
						0,  // offset
						transformationMatrix4x4BufferSize  // range
					),
					vk::DescriptorBufferInfo(
						normalMatrix4x3Buffer.get(),  // buffer
						0,  // offset
						normalMatrix4x3BufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				transformationFiveMatricesUsingGSDescriptorSet,  // dstSet
				4,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eUniformBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						viewAndProjectionMatricesUniformBuffer.get(),  // buffer
						0,  // offset
						viewAndProjectionMatricesBufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				transformationFiveMatricesUsingGSAndAttributesDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				2,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,2>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						sameMatrixBuffer.get(),  // buffer
						0,  // offset
						transformationMatrix4x4BufferSize  // range
					),
					vk::DescriptorBufferInfo(
						normalMatrix4x3Buffer.get(),  // buffer
						0,  // offset
						normalMatrix4x3BufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				transformationFiveMatricesUsingGSAndAttributesDescriptorSet,  // dstSet
				2,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eUniformBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						viewAndProjectionMatricesUniformBuffer.get(),  // buffer
						0,  // offset
						viewAndProjectionMatricesBufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				phongTexturedThreeDMatricesUsingGSAndAttributesDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eStorageBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						sameDMatrixBuffer.get(),  // buffer
						0,  // offset
						transformationDMatrix4x4BufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				phongTexturedThreeDMatricesUsingGSAndAttributesDescriptorSet,  // dstSet
				1,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eUniformBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						viewAndProjectionDMatricesUniformBuffer.get(),  // buffer
						0,  // offset
						viewAndProjectionDMatricesBufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				phongTexturedDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				3,  // descriptorCount
				vk::DescriptorType::eUniformBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,3>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						materialUniformBuffer.get(),  // buffer
						0,  // offset
						materialUniformBufferSize  // range
					),
					vk::DescriptorBufferInfo(
						globalLightUniformBuffer.get(),  // buffer
						0,  // offset
						globalLightUniformBufferSize  // range
					),
					vk::DescriptorBufferInfo(
						lightUniformBuffer.get(),  // buffer
						0,  // offset
						lightUniformBufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				phongTexturedNotPackedDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				3,  // descriptorCount
				vk::DescriptorType::eUniformBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,3>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						materialNotPackedUniformBuffer.get(),  // buffer
						0,  // offset
						materialNotPackedUniformBufferSize  // range
					),
					vk::DescriptorBufferInfo(
						globalLightUniformBuffer.get(),  // buffer
						0,  // offset
						globalLightUniformBufferSize  // range
					),
					vk::DescriptorBufferInfo(
						lightNotPackedUniformBuffer.get(),  // buffer
						0,  // offset
						lightNotPackedUniformBufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				phongTexturedDescriptorSet,  // dstSet
				3,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eCombinedImageSampler,  // descriptorType
				array<vk::DescriptorImageInfo,1>{  // pImageInfo
					vk::DescriptorImageInfo(
						trilinearSampler.get(),      // sampler
						singleTexelImageView.get(),  // imageView
						vk::ImageLayout::eShaderReadOnlyOptimal  // imageLayout
					),
				}.data(),
				nullptr,  // pBufferInfo
				nullptr  // pTexelBufferView
			},
			{
				phongTexturedNotPackedDescriptorSet,  // dstSet
				3,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eCombinedImageSampler,  // descriptorType
				array<vk::DescriptorImageInfo,1>{  // pImageInfo
					vk::DescriptorImageInfo(
						trilinearSampler.get(),      // sampler
						singleTexelImageView.get(),  // imageView
						vk::ImageLayout::eShaderReadOnlyOptimal  // imageLayout
					),
				}.data(),
				nullptr,  // pBufferInfo
				nullptr  // pTexelBufferView
			},
			{
				threeUniformFSDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				3,  // descriptorCount
				vk::DescriptorType::eUniformBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,3>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						materialUniformBuffer.get(),  // buffer
						0,  // offset
						materialUniformBufferSize  // range
					),
					vk::DescriptorBufferInfo(
						globalLightUniformBuffer.get(),  // buffer
						0,  // offset
						globalLightUniformBufferSize  // range
					),
					vk::DescriptorBufferInfo(
						lightUniformBuffer.get(),  // buffer
						0,  // offset
						lightUniformBufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
			{
				allInOneLightingUniformDescriptorSet,  // dstSet
				0,  // dstBinding
				0,  // dstArrayElement
				1,  // descriptorCount
				vk::DescriptorType::eUniformBuffer,  // descriptorType
				nullptr,  // pImageInfo
				array<vk::DescriptorBufferInfo,1>{  // pBufferInfo
					vk::DescriptorBufferInfo(
						allInOneLightingUniformBuffer.get(),  // buffer
						0,  // offset
						allInOneLightingUniformBufferSize  // range
					),
				}.data(),
				nullptr  // pTexelBufferView
			},
		}},
		nullptr  // descriptorCopies
	);


	// timestamp pool
	timestampPool=
		device->createQueryPoolUnique(
			vk::QueryPoolCreateInfo(
				vk::QueryPoolCreateFlags(),  // flags
				vk::QueryType::eTimestamp,   // queryType
				uint32_t(tests.size())*2,    // queryCount
				vk::QueryPipelineStatisticFlags()  // pipelineStatistics
			)
		);
}


/// Queue one frame for rendering
static void frame()
{
	// reset command pool
	// and begin command buffer
	device->resetCommandPool(commandPool.get(), vk::CommandPoolResetFlags());
	commandBuffer->begin(
		vk::CommandBufferBeginInfo(
			vk::CommandBufferUsageFlagBits::eOneTimeSubmit,  // flags
			nullptr  // pInheritanceInfo
		)
	);
	commandBuffer->resetQueryPool(timestampPool.get(), 0, uint32_t(tests.size())*2);

	// shuffle tests
	// to run them in different order each time
	// except the first test doing warm up;
	// it avoids some problems on Radeons when one test might cause
	// following test to perform poorly probably because some parts of the GPU are
	// switched into powersaving states because of not high enough load)
	minstd_rand rnd;
	shuffle(shuffledTests.begin()+1, shuffledTests.end(), rnd);

	// record all tests
	for(size_t j=0,c=tests.size(); j<c; j++)
		shuffledTests[j]->func(commandBuffer.get(), shuffledTests[j]->timestampIndex, shuffledTests[j]->groupVariable);

	// end command buffer
	commandBuffer->end();

	// submit work
	graphicsQueue.submit(
		vk::SubmitInfo(
			0,nullptr,nullptr,  // waitSemaphoreCount+pWaitSemaphores+pWaitDstStageMask
			1,&commandBuffer.get(),  // commandBufferCount+pCommandBuffers
			0,nullptr  // signalSemaphoreCount+pSignalSemaphores
		),
		fence.get()
	);

	// wait for work to complete
	vk::Result r=
		device->waitForFences(
			fence.get(),         // fences (vk::ArrayProxy)
			VK_TRUE,       // waitAll
			uint64_t(3e9)  // timeout (3s)
		);
	if(r==vk::Result::eTimeout)
		throw std::runtime_error("GPU timeout. Task is probably hanging.");
	device->resetFences(
		fence.get()
	);
}


static void testMemoryAllocationPerformance(vk::BufferCreateFlags bufferFlags,unsigned virtualSpaceMultiplier)
{
	auto test=
		[](size_t numBuffers,size_t numMemoryObjectsPerBuffer,size_t memoryObjectSize,unsigned virtualSpaceMultiplier,
		   vk::BufferCreateFlags bufferFlags)->tuple<double,double,double> {

			// create buffers
			vector<vk::UniqueBuffer> bufferList;
			bufferList.reserve(numBuffers);
			auto startTime=chrono::high_resolution_clock::now();
			for(size_t i=0; i<numBuffers; i++)
				bufferList.emplace_back(
					device->createBufferUnique(
						vk::BufferCreateInfo(
							bufferFlags,                  // flags
							numMemoryObjectsPerBuffer*memoryObjectSize*virtualSpaceMultiplier,  // size
							vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eTransferDst,  // usage
							vk::SharingMode::eExclusive,  // sharingMode
							0,                            // queueFamilyIndexCount
							nullptr                       // pQueueFamilyIndices
						)
					)
				);
			double bufferCreationTime=chrono::duration<double>(chrono::high_resolution_clock::now()-startTime).count();

			// allocate memory
			vector<vk::UniqueDeviceMemory> memoryList;
			memoryList.reserve(numBuffers*numMemoryObjectsPerBuffer);
			startTime=chrono::high_resolution_clock::now();
			for(size_t i=0; i<numBuffers; i++)
				for(size_t j=0; j<numMemoryObjectsPerBuffer; j++)
					memoryList.emplace_back(
						get<0>(
							allocateMemory(
								bufferList[i].get(),
								vk::MemoryPropertyFlagBits::eDeviceLocal,
								numMemoryObjectsPerBuffer*virtualSpaceMultiplier
							)
						)
					);
			double memoryCreationTime=chrono::duration<double>(chrono::high_resolution_clock::now()-startTime).count();

			double bindTime;
			if(!(bufferFlags&vk::BufferCreateFlagBits::eSparseBinding)) {

				// bind non-sparse memory
				assert(numMemoryObjectsPerBuffer==1 && "Non-sparse buffers support only one attached DeviceMemory object.");

				startTime=chrono::high_resolution_clock::now();
				for(size_t i=0; i<numBuffers; i++)
					device->bindBufferMemory(
						bufferList[i].get(),  // buffer
						memoryList[i].get(),  // memory
						0  // memoryOffset
					);
				bindTime=chrono::duration<double>(chrono::high_resolution_clock::now()-startTime).count();
			}
			else {

				// fill SparseMemoryBind structures
				vector<vk::SparseMemoryBind> memoryBinds;
				memoryBinds.reserve(numBuffers*numMemoryObjectsPerBuffer);
				for(size_t i=0,idx=0; i<numBuffers; i++)
					for(size_t j=0; j<numMemoryObjectsPerBuffer; j++,idx++)
						memoryBinds.emplace_back(
							j*memoryObjectSize,  // resourceOffset
							memoryObjectSize,  // size
							memoryList[idx].get(),  // memory
							0,  // memoryOffset
							vk::SparseMemoryBindFlags()  // flags
						);

				// fill SparseBufferMemoryBindInfo structures
				vector<vk::SparseBufferMemoryBindInfo> bufferBinds;
				bufferBinds.reserve(numBuffers);
				for(size_t i=0; i<numBuffers; i++)
					bufferBinds.emplace_back(
						bufferList[i].get(),  // buffer
						uint32_t(numMemoryObjectsPerBuffer),  // bindCount
						memoryBinds.data()+(i*numMemoryObjectsPerBuffer)  // pBinds
					);

				// call bindSparse
				startTime=chrono::high_resolution_clock::now();
				sparseQueue.bindSparse(
					vk::BindSparseInfo(
						nullptr,  // waitSemaphores
						bufferBinds,  // bufferBinds
						nullptr,  // imageOpaqueBinds
						nullptr,  // imageBinds
						nullptr  // signalSemaphores
					),
					vk::Fence()
				);
				sparseQueue.waitIdle();
				bindTime=chrono::duration<double>(chrono::high_resolution_clock::now()-startTime).count();
			}

			// return time in seconds as double
			return make_tuple(bufferCreationTime,memoryCreationTime,bindTime);
		};

	// perform test
	unsigned repeatCount=(longTest)?30:3;
	auto testData=
		!(bufferFlags&vk::BufferCreateFlagBits::eSparseBinding)
			? vector{
				tuple{1,1,   1}, // to warm up caches, will be not included in results
				tuple{1,1,   1}, // to warm up caches, will be not included in results
				tuple{1,1,   1},
				tuple{1,1,   2},
				tuple{1,1,  32},
				tuple{1,1, 512},
				tuple{1,1,1024},
				tuple{32,1, 32},
				tuple{1024,1,1},
			}
			: vector{
				// numBuffers, memObjPerBuffer, numMemBlocksPerMemObj
				tuple{1,1,   1}, // to warm up caches, this will not be included in the results
				tuple{1,1,   1}, // to warm up caches, this will not be included in the results
				tuple{1,1,   1},
				tuple{1,1,   2},
				tuple{1,1,   8},
				tuple{1,1,  32},
				tuple{1,1,  64},
				tuple{1,1, 128},
				tuple{1,1, 512},
				tuple{1,1,1024},
				tuple{1,1, 256},
				tuple{1,2, 128},
				tuple{1,16, 16},
				tuple{1,128, 2},
				tuple{1,256, 1},
				tuple{2,1, 128},
				tuple{16,1, 16},
				tuple{4,4,  16},
			};
	vector<double> bufferCreationResults(testData.size(),numeric_limits<double>::max());
	vector<double> memoryCreationResults(testData.size(),numeric_limits<double>::max());
	vector<double> bindResults(testData.size(),numeric_limits<double>::max());
	auto startTime=chrono::high_resolution_clock::now();
	for(size_t j=0; j<repeatCount; j++)
		for(size_t i=0; i<testData.size(); i++) {
			auto& d=testData[i];
			auto [bufferCreationTime,memoryCreationTime,bindTime]=
				test(get<0>(d),get<1>(d),get<2>(d)*sparseBlockSize,virtualSpaceMultiplier,bufferFlags);
			bufferCreationResults[i]=min(bufferCreationTime,bufferCreationResults[i]);
			memoryCreationResults[i]=min(memoryCreationTime,memoryCreationResults[i]);
			bindResults[i]=min(bindTime,bindResults[i]);
		}
	double totalMeasurementTime=chrono::duration<double>(chrono::high_resolution_clock::now()-startTime).count();

	auto printTime=
		[](double t) -> string {
			stringstream ss;
			ss<<setprecision(3);
			if(t>=1)
				ss<<t<<"s";
			else if(t>=1e-3)
				ss<<t*1e3<<"ms";
			else if(t>=1e-6)
				ss<<t*1e6<<"us";
			else
				ss<<t*1e9<<"ns";
			return ss.str();
		};
	auto printAmountOfMemory=
		[](size_t value) -> string {
			stringstream ss;
			ss<<setprecision(3);
			if(value<1000)
				ss<<value<<"B";
			else {
				float f=float(value)/1024;
				if(f<1000.f)
					ss<<f<<"KiB";
				else {
					f/=1024;
					if(f<1000.f)
						ss<<f<<"MiB";
					else {
						f/=1024;
						if(f<1000.f)
							ss<<f<<"GiB";
						else
							ss<<f/1024<<"TiB";
					}
				}
			}
			return ss.str();
		};


	// print results
#if 1
	cout<<"  number of      total   one   costOfMem  one   costOfMem  one    memory  total"<<endl;
	cout<<"  buffers/       memory buffer  block on memObj  block on buffer  block   cost"<<endl;
	cout<<"  memObjPerBuf/  alloc- create  buffer   create  memObj    bind   bind    ofMem"<<endl;
	cout<<"  memBlocksPerObj ated   time  creation   time  creation   time   time    block"<<endl;
	for(size_t i=2; i<testData.size(); i++) {
		size_t numBuffers=get<0>(testData[i]);
		size_t numMemObjPerBuffer=get<1>(testData[i]);
		size_t numMemBlocksPerMemObj=get<2>(testData[i]);
		cout<<setw(4)<<numBuffers<<" "
		    <<setw(4)<<numMemObjPerBuffer<<" "
			<<setw(4)<<numMemBlocksPerMemObj<<" "
			<<setw(7)<<printAmountOfMemory(numBuffers*numMemObjPerBuffer*numMemBlocksPerMemObj*sparseBlockSize)<<" "
			<<setw(7)<<printTime(bufferCreationResults[i]/numBuffers)<<" "
			<<setw(7)<<printTime(bufferCreationResults[i]/(numBuffers*numMemObjPerBuffer*numMemBlocksPerMemObj))<<"  "
			<<setw(7)<<printTime(memoryCreationResults[i]/(numBuffers*numMemObjPerBuffer))<<" "
			<<setw(7)<<printTime(memoryCreationResults[i]/(numBuffers*numMemObjPerBuffer*numMemBlocksPerMemObj))<<" "
			<<setw(7)<<printTime(bindResults[i]/numBuffers)<<" "
			<<setw(7)<<printTime(bindResults[i]/(numBuffers*numMemObjPerBuffer*numMemBlocksPerMemObj))<<" "
			<<setw(7)<<printTime((bufferCreationResults[i]+memoryCreationResults[i]+bindResults[i])/
			                     (numBuffers*numMemObjPerBuffer*numMemBlocksPerMemObj))<<endl;
	}
#else
	cout<<"  num  numMem  mem    time of   costOfMem  time of  costOfMem  buffer  memory"<<endl;
	cout<<"   of  ObjPer blocks  a buffer   block on   memObj  block on    bind   block"<<endl;
	cout<<"  bufs buffer perObj  creation  bufCreate  creation memCreate   time  bindTime"<<endl;
	for(size_t i=2; i<testData.size(); i++) {
		size_t numBuffers=get<0>(testData[i]);
		size_t numMemObjPerBuffer=get<1>(testData[i]);
		size_t numMemBlocksPerMemObj=get<2>(testData[i]);
		cout<<setw(5)<<numBuffers<<"  "
		    <<setw(5)<<numMemObjPerBuffer<<"  "
			<<setw(5)<<numMemBlocksPerMemObj<<"   "
			<<setw(7)<<printTime(bufferCreationResults[i]/numBuffers)<<"  "
			<<setw(8)<<printTime(bufferCreationResults[i]/(numBuffers*numMemObjPerBuffer*numMemBlocksPerMemObj))<<"    "
			<<setw(7)<<printTime(memoryCreationResults[i]/(numBuffers*numMemObjPerBuffer))<<"  "
			<<setw(7)<<printTime(memoryCreationResults[i]/(numBuffers*numMemObjPerBuffer*numMemBlocksPerMemObj))<<"    "
			<<setw(7)<<printTime(bindResults[i]/numBuffers)<<"  "
			<<setw(7)<<printTime(bindResults[i]/(numBuffers*numMemObjPerBuffer*numMemBlocksPerMemObj))<<endl;
	}
#endif
	cout<<"Total measurement time: "<<printTime(totalMeasurementTime)<<endl;
}


/// main function of the application
int main(int argc,char** argv)
{
	// print header
	cout << endl;
	cout << appName << " (" << VK_VERSION_MAJOR(appVersion) << "." << VK_VERSION_MINOR(appVersion) << "."
	     << VK_VERSION_PATCH(appVersion) << ") tests various performance characteristics of Vulkan devices.\n" << endl;

	// catch exceptions
	// (vulkan.hpp fuctions throw if they fail)
	try {

		// process command-line arguments
		bool printHelp = false;
		int physicalDeviceIndex = -1;
		string deviceNameFilter;
		for(int i=1; i<argc; i++) {

			if(argv[i] == nullptr || argv[i][0] == 0)
				continue;

			// parse options starting with '-'
			if(argv[i][0] == '-') {

				if(strcmp(argv[i], "--long") == 0)  longTest = true;
				else if(strcmp(argv[i], "--minimal") == 0)  minimalTest = true;
				else if(strcmp(argv[i], "--sparse-none") == 0)  sparseMode = SPARSE_NONE;
				else if(strcmp(argv[i], "--sparse-binding") == 0)  sparseMode = SPARSE_BINDING;
				else if(strcmp(argv[i], "--sparse-residency") == 0)  sparseMode = SPARSE_RESIDENCY;
				else if(strcmp(argv[i], "--sparse-residency-aliased") == 0)  sparseMode = SPARSE_RESIDENCY_ALIASED;
				else if(strcmp(argv[i], "--debug") == 0)  debug = true;
				else if(strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)  printHelp = true;
				else {
					cout << "Invalid argument: " << argv[i] << endl;
					printHelp = true;
				}
			}
			// parse whatever does not start with '-'
			else {
				// parse numbers
				if(argv[i][0] >= '0' && argv[i][0] <= '9') {
					char* e = nullptr;
					physicalDeviceIndex = strtoul(argv[i], &e, 10);
					if(e == nullptr || *e != 0) {
						cout << "Invalid parameter \"" << argv[i] << "\"" << endl;
						printHelp = true;
					}
				}
				// parse text
				else
					deviceNameFilter = argv[i];
			}
		}

		// print usage info and exit
		if(printHelp) {
			cout << "Usage:\n"
			        "   " << appName << " [deviceNameFilter] [deviceIndex] [options]\n"
			        "   --long - perform long test; testing time is extended to " << int(longTestTime+0.5) << " seconds\n"
			        "            from the default of " << int(standardTestTime+0.5) << " seconds\n"
			        "   --minimal - perform minimal test; for debugging purposes,\n"
			        "               number of triangles used for testing is lagerly\n"
			        "               reduced, making measurements possibly very imprecise\n"
			        "   --sparse-none - no sparse mode is used during the main test;\n"
			        "                   this is the default\n"
			        "   --sparse-binding - sparse binding mode is used during the main test\n"
			        "   --sparse-residency - sparse residency mode is used during the main test\n"
			        "   --sparse-residency-aliased - sparse residency aliased mode is used\n"
			        "                                during the main test\n"
			        "   --debug or -d - print additional debug info\n"
			        "   --help or -h - prints the usage information" << endl;
			exit(99);
		}

		// init Vulkan and open window,
		// give physical device index as parameter
		init(deviceNameFilter, physicalDeviceIndex);

		// create test objects
		initTests();

		// recreate swapchain
		resizeFramebuffer(defaultFramebufferExtent);

		auto startTime=chrono::steady_clock::now();

		while(true){

			// render frame
			frame();

			// read timestamps
			vector<uint64_t> timestamps(tests.size()*2);
			vk::Result r=device->getQueryPoolResults(
				timestampPool.get(),  // queryPool
				0,                    // firstQuery
				uint32_t(tests.size())*2,  // queryCount
				tests.size()*2*sizeof(uint64_t),  // dataSize
				timestamps.data(),    // pData
				sizeof(uint64_t),     // stride
				vk::QueryResultFlagBits::e64|vk::QueryResultFlagBits::eWait  // flags
			);
			if(r!=vk::Result::eSuccess)
				throw std::runtime_error("vkGetQueryPoolResults() did not finish with VK_SUCCESS result.");
			size_t i=0;
			for(Test& t : tests) {
				if(t.enabled)
					t.renderingTimes.emplace_back(timestamps[i+1]-timestamps[i]);
				i+=2;
			}

			// verify that tests did not overlap
			uint64_t v = 0;
			for(auto it=shuffledTests.begin(); it!=shuffledTests.end(); it++)
				if((*it)->enabled) {
					uint32_t i = (*it)->timestampIndex;
					if(v > timestamps[i])
						throw std::runtime_error("Tests ran in parallel.");
					if(timestamps[i] > timestamps[i+1])
						throw std::runtime_error("Tests ran in parallel.");
					v = timestamps[i+1];
				}

			// print the result at the end
			double totalMeasurementTime=chrono::duration<double>(chrono::steady_clock::now()-startTime).count();
			if(totalMeasurementTime>((longTest)?longTestTime:standardTestTime)) {
				cout<<"Triangle throughput:"<<endl;
				for(size_t i=0; i<tests.size(); i++) {
					Test& t = tests[i];
					if(t.type==Test::Type::VertexThroughput) {
						if(i!=0 && tests[i].groupText && tests[i-1].groupText!=tests[i].groupText)
							cout << tests[i].groupText << endl;
						if(t.enabled) {
							sort(t.renderingTimes.begin(), t.renderingTimes.end());
							double time_ns = t.renderingTimes[(t.renderingTimes.size()-1)/2] * timestampPeriod_ns;
							cout << t.text << double(numTriangles)/time_ns*1e9/1e6 << " million triangles/s" << endl;
						}
						else
							cout << t.text << " not supported" << endl;
					}
				}
				cout<<"\nFragment throughput:"<<endl;
				size_t numScreenFragments=size_t(framebufferExtent.width)*framebufferExtent.height;
				for(size_t i=0; i<tests.size(); i++) {
					Test& t = tests[i];
					if(t.type==Test::Type::FragmentThroughput) {
						if(i!=0 && tests[i].groupText && tests[i-1].groupText!=tests[i].groupText)
							cout << tests[i].groupText << endl;
						if(t.enabled) {
							sort(t.renderingTimes.begin(), t.renderingTimes.end());
							double time_ns = t.renderingTimes[(t.renderingTimes.size()-1)/2] * timestampPeriod_ns;
							cout << t.text << double(numScreenFragments)*t.numRenderedItems/time_ns*1e9/1e9 << " * 1e9 per second" << endl;
						#if 0 // tuning of tests to not take too long
							cout << "      measurement time: " << time_ns/1e6 << "ms" << endl;
						#endif
						}
						else
							cout << t.text << " not supported" << endl;
					}
				}
				cout<<"\nTransfer throughput:"<<endl;
				size_t numTransferTests = 0;
				for(size_t i=0; i<tests.size(); i++) {
					Test& t = tests[i];
					if(t.type==Test::Type::TransferThroughput) {
						if(i!=0 && tests[i].groupText && tests[i-1].groupText!=tests[i].groupText)
							cout << tests[i].groupText << endl;
						if(!t.renderingTimes.empty()) {
							sort(t.renderingTimes.begin(), t.renderingTimes.end());
							double time_ns = t.renderingTimes[(t.renderingTimes.size()-1)/2] * timestampPeriod_ns;
							cout << t.text << time_ns/t.numTransfers << "ns per transfer ("
							     << double(t.numTransfers*t.transferSize)/time_ns*1e9/1024/1024/1024 << " GiB/s)" << endl;
							numTransferTests = t.renderingTimes.size();
						}
						else
							cout << t.text << "not run" << endl;
					}
				}
				cout << "\nNumber of measurements of vertex and fragment tests: " << tests.front().renderingTimes.size() << endl;
				cout << "Number of measurements of transfer tests: " << numTransferTests << endl;
				cout << "Total time of all measurements: " << totalMeasurementTime << " seconds" << endl;
				cout << endl;

				// device with sparse memory support
				vk::UniqueDevice sparseDevice;
				vk::UniqueDevice sparseResidencyDevice;
				vk::PhysicalDeviceFeatures physicalFeatures=physicalDevice.getFeatures();
				if(physicalFeatures.sparseBinding) {
					vk::PhysicalDeviceFeatures enabledFeatures;
					enabledFeatures.setSparseBinding(true);
					sparseDevice=
						physicalDevice.createDeviceUnique(
							vk::DeviceCreateInfo{
								vk::DeviceCreateFlags(),  // flags
								(sparseQueueFamily==graphicsQueueFamily)  // queueCreateInfoCount
									?uint32_t(1):uint32_t(2),
								array<const vk::DeviceQueueCreateInfo,2>{  // pQueueCreateInfos
									vk::DeviceQueueCreateInfo{
										vk::DeviceQueueCreateFlags(),
										graphicsQueueFamily,
										1,
										&(const float&)1.f,
									},
									vk::DeviceQueueCreateInfo{
										vk::DeviceQueueCreateFlags(),
										sparseQueueFamily,
										1,
										&(const float&)1.f,
									},
								}.data(),
								0,nullptr,  // no layers
								0,        // number of enabled extensions
								nullptr,  // enabled extension names
								&enabledFeatures,  // enabled features
							}
						);
					if(physicalFeatures.sparseResidencyBuffer) {
						enabledFeatures.setSparseResidencyBuffer(true);
						enabledFeatures.setSparseResidencyAliased(physicalFeatures.sparseResidencyAliased);
						sparseResidencyDevice=
							physicalDevice.createDeviceUnique(
								vk::DeviceCreateInfo{
									vk::DeviceCreateFlags(),  // flags
									(sparseQueueFamily==graphicsQueueFamily)  // queueCreateInfoCount
										?uint32_t(1):uint32_t(2),
									array<const vk::DeviceQueueCreateInfo,2>{  // pQueueCreateInfos
										vk::DeviceQueueCreateInfo{
											vk::DeviceQueueCreateFlags(),
											graphicsQueueFamily,
											1,
											&(const float&)1.f,
										},
										vk::DeviceQueueCreateInfo{
											vk::DeviceQueueCreateFlags(),
											sparseQueueFamily,
											1,
											&(const float&)1.f,
										},
									}.data(),
									0,nullptr,  // no layers
									0,        // number of enabled extensions
									nullptr,  // enabled extension names
									&enabledFeatures,  // enabled features
								}
							);
					}
				}

				// print memory alignment
				cout<<"Graphics memory allocation properties"<<endl;
				cout<<"Standard buffer alignment:  "
					<<getMemoryAlignment(device.get(),1,vk::BufferCreateFlags())<<endl;

				// set sparse block variables
				if(sparseDevice) {
					sparseBlockSize=getMemoryAlignment(sparseDevice.get(),1,vk::BufferCreateFlagBits::eSparseBinding);
					memoryBlockSize=sparseBlockSize;
					memoryBlockMask=~(sparseBlockSize-1);
					cout<<"Graphics memory page size:  "<<sparseBlockSize<<endl;
				}
				else {
					cout<<"Graphics memory page size: Could not be retrieved."<<endl;
					cout<<"   No sparse memory support. Further measurements will report\n"
					      "   results like the graphics memory page size would be 64KiB."<<endl;
					sparseBlockSize=65536;
					memoryBlockSize=65536;
					memoryBlockSize=~0xffff;
				}

				// sparse properties
				if(sparseDevice) {
					vk::PhysicalDeviceFeatures physicalFeatures=physicalDevice.getFeatures();
					cout<<"Sparse address space:       0x"<<std::hex<<physicalDeviceProperties.limits.sparseAddressSpaceSize<<std::dec<<
						" ("<<(((physicalDeviceProperties.limits.sparseAddressSpaceSize+512)/1024+512)/1024+512)/1024<<"GiB)"<<endl;
					cout<<"Sparse binding:             "<<physicalFeatures.sparseBinding<<endl;
					cout<<"Sparse residency buffer:    "<<physicalFeatures.sparseResidencyBuffer<<endl;
					cout<<"Sparse residency image2D:   "<<physicalFeatures.sparseResidencyImage2D<<endl;
					cout<<"Sparse residency 4samples:  "<<physicalFeatures.sparseResidency4Samples<<endl;
					cout<<"Sparse residency aliased:   "<<physicalFeatures.sparseResidencyAliased<<endl;
					cout<<"Sparse buffer page size:    ";
					if(physicalFeatures.sparseResidencyBuffer)
						cout<<getMemoryAlignment(sparseResidencyDevice.get(),1,
							vk::BufferCreateFlagBits::eSparseBinding|vk::BufferCreateFlagBits::eSparseResidency|
							(physicalFeatures.sparseResidencyAliased?vk::BufferCreateFlagBits::eSparseAliased:vk::BufferCreateFlags()))<<endl;
					else
						cout<<"no sparse residency buffer support"<<endl;
					vk::SparseImageFormatProperties imageFormatProperties=
						[]() {
							auto l=physicalDevice.getSparseImageFormatProperties(vk::Format::eR8G8B8A8Uint,vk::ImageType::e2D,vk::SampleCountFlagBits::e1,vk::ImageUsageFlagBits::eSampled,vk::ImageTiling::eOptimal);
							for(vk::SparseImageFormatProperties& p : l)
								if(p.aspectMask&vk::ImageAspectFlagBits::eColor)
									return p;
							return vk::SparseImageFormatProperties{ {},{0,0,0},{} };
						}();
					cout<<"Sparse image (R8G8B8A8) block: "<<imageFormatProperties.imageGranularity.width<<"x"<<imageFormatProperties.imageGranularity.height<<endl;
				}

				// perform non-sparse test
				cout<<endl;
				cout<<"Standard memory allocation performance"<<endl;
				testMemoryAllocationPerformance(vk::BufferCreateFlags(),1);

				cout<<endl;
				cout<<"Sparse memory allocation performance"<<endl;
				if(!sparseDevice)
					cout<<"   No sparse memory support."<<endl;
				else {

					// replace device by sparseDevice
					device.swap(sparseDevice);

					// get queues
					graphicsQueue=device->getQueue(graphicsQueueFamily,0);
					sparseQueue=device->getQueue(sparseQueueFamily,0);

					// perform test
					testMemoryAllocationPerformance(vk::BufferCreateFlagBits::eSparseBinding,1);

					// return original device back
					device.swap(sparseDevice);
				}
				cout<<endl;

				break;
			}

		}

		device->waitIdle();

	// catch exceptions
	} catch(vk::Error& e) {
		cout<<"Failed because of Vulkan exception: "<<e.what()<<endl;
	} catch(exception& e) {
		cout<<"Failed because of exception: "<<e.what()<<endl;
	} catch(...) {
		cout<<"Failed because of unspecified exception."<<endl;
	}

	// wait device idle, particularly, if there was an exception and device is busy
	// (device must be idle before destructors of buffers and other stuff are called)
	if(device) {
		try {
			device->waitIdle();
		} catch(vk::Error&) {
			// ignore all Vulkan exceptions (especially vk::DeviceLostError)
		}
	}

	return 0;
}
