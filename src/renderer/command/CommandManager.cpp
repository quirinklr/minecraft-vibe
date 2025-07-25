#include "CommandManager.h"
#include <stdexcept>
#include <array>
#include "../../math/Ivec3Less.h"

CommandManager::CommandManager(const DeviceContext &deviceContext, const SwapChainContext &swapChainContext, const PipelineCache &pipelineCache)
    : m_DeviceContext(deviceContext), m_SwapChainContext(swapChainContext), m_PipelineCache(pipelineCache)
{
    createCommandPool();
    createCommandBuffers();
}

CommandManager::~CommandManager() {}

void CommandManager::createCommandPool()
{
    DeviceContext::QueueFamilyIndices queueFamilyIndices = m_DeviceContext.findQueueFamilies(m_DeviceContext.getPhysicalDevice());
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    VkCommandPool pool;
    if (vkCreateCommandPool(m_DeviceContext.getDevice(), &poolInfo, nullptr, &pool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
    m_CommandPool = VulkanHandle<VkCommandPool, CommandPoolDeleter>(pool, {m_DeviceContext.getDevice()});
}

void CommandManager::createCommandBuffers()
{
    m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_CommandPool.get();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();
    if (vkAllocateCommandBuffers(m_DeviceContext.getDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void CommandManager::recordCommandBuffer(uint32_t imageIndex, uint32_t currentFrame,
                                         const std::vector<std::pair<const Chunk *, int>> &chunksToRender,
                                         const std::vector<VkDescriptorSet> &descriptorSets,
                                         VkBuffer crosshairVertexBuffer, bool wireframe)
{
    VkCommandBuffer cb = m_CommandBuffers[currentFrame];

    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkBeginCommandBuffer(cb, &beginInfo);

    std::array<VkClearValue, 2> clear{};
    clear[0].color = {{0.5f, 0.7f, 1.0f, 1.0f}};
    clear[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo rp{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    rp.renderPass = m_SwapChainContext.getRenderPass();
    rp.framebuffer = m_SwapChainContext.getFramebuffers()[imageIndex].get();
    rp.renderArea = {{0, 0}, m_SwapChainContext.getSwapChainExtent()};
    rp.clearValueCount = clear.size();
    rp.pClearValues = clear.data();

    vkCmdBeginRenderPass(cb, &rp, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport vp{0, 0,
                  float(m_SwapChainContext.getSwapChainExtent().width),
                  float(m_SwapChainContext.getSwapChainExtent().height),
                  0, 1};
    VkRect2D sc{{0, 0}, m_SwapChainContext.getSwapChainExtent()};
    vkCmdSetViewport(cb, 0, 1, &vp);
    vkCmdSetScissor(cb, 0, 1, &sc);

    VkPipeline mainPipe = wireframe
                              ? m_PipelineCache.getWireframePipeline()
                              : m_PipelineCache.getGraphicsPipeline();

    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, mainPipe);

    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, mainPipe);
    vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_PipelineCache.getGraphicsPipelineLayout(),
                            0, 1, &descriptorSets[currentFrame], 0, nullptr);

    for (const auto &[chunk, lod] : chunksToRender)
    {
        const ChunkMesh *mesh = chunk->getMesh(lod);

        if (!mesh || mesh->indexCount == 0)
            continue;

        VkBuffer vertexBuffers[] = {mesh->vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cb, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(cb, mesh->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdPushConstants(cb, m_PipelineCache.getGraphicsPipelineLayout(),
                           VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4),
                           &chunk->getModelMatrix());

        vkCmdDrawIndexed(cb, mesh->indexCount, 1, 0, 0, 0);
    }

    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      m_PipelineCache.getCrosshairPipeline());
    vkCmdSetLineWidth(cb, 2.5f);
    VkDeviceSize z = 0;
    vkCmdBindVertexBuffers(cb, 0, 1, &crosshairVertexBuffer, &z);
    vkCmdDraw(cb, 4, 1, 0, 0);

    vkCmdEndRenderPass(cb);
    vkEndCommandBuffer(cb);
}
