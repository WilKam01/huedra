#pragma once
#include "platform/cocoa/window.hpp"
#include "platform/metal/config.hpp"
#include "platform/metal/render_target.hpp"

#include <atomic>
#include <condition_variable>
#include <thread>

namespace huedra {

struct MetalDrawableFetchInfo
{
    MetalDrawableFetchInfo() = default;
    virtual ~MetalDrawableFetchInfo() = default;

    MetalDrawableFetchInfo(const MetalDrawableFetchInfo& rhs)
        : runningThread(rhs.runningThread), alreadyFetchedDrawable(rhs.alreadyFetchedDrawable.load()),
          fetchDrawableThread(rhs.fetchDrawableThread)
    {}

    MetalDrawableFetchInfo& operator=(const MetalDrawableFetchInfo& rhs)
    {
        if (this != &rhs)
        {
            runningThread = rhs.runningThread;
            alreadyFetchedDrawable = rhs.alreadyFetchedDrawable.load();
            fetchDrawableThread = rhs.fetchDrawableThread;
        }
        return *this;
    }

    MetalDrawableFetchInfo(MetalDrawableFetchInfo&& rhs)
        : runningThread(rhs.runningThread), alreadyFetchedDrawable(rhs.alreadyFetchedDrawable.load()),
          fetchDrawableThread(rhs.fetchDrawableThread)
    {}

    MetalDrawableFetchInfo& operator=(MetalDrawableFetchInfo&& rhs)
    {
        if (this != &rhs)
        {
            runningThread = rhs.runningThread;
            alreadyFetchedDrawable = rhs.alreadyFetchedDrawable.load();
            fetchDrawableThread = rhs.fetchDrawableThread;
        }
        return *this;
    }

    bool runningThread{false};
    std::atomic_bool alreadyFetchedDrawable{false};
    std::thread* fetchDrawableThread{nullptr};
    std::mutex conditionMutex;
    std::condition_variable condition;
};

class MetalSwapchain
{
public:
    MetalSwapchain() = default;
    virtual ~MetalSwapchain() = default;

    MetalSwapchain(const MetalSwapchain& rhs) = default;
    MetalSwapchain& operator=(const MetalSwapchain& rhs) = default;
    MetalSwapchain(MetalSwapchain&& rhs) = default;
    MetalSwapchain& operator=(MetalSwapchain&& rhs) = default;

    void init(id<MTLDevice> device, id<MTLCommandQueue> commandQueue, WindowCocoa* window, bool renderDepth);
    void cleanup();

    void aquireNextDrawable();
    void releaseDrawable();

    CAMetalLayer* getLayer() const { return m_layer; }
    id<CAMetalDrawable> getDrawable() const { return m_currentDrawable; }
    void setTextureRendered() { m_textureRendered = true; }

private:
    id<MTLDevice> m_device;
    id<MTLCommandQueue> m_commandQueue;
    WindowCocoa* m_window;
    CAMetalLayer* m_layer;
    id<CAMetalDrawable> m_currentDrawable;
    dispatch_semaphore_t m_presentSemaphore;
    bool m_textureRendered{false};

    MetalDrawableFetchInfo m_threadInfo;

    MetalRenderTarget m_renderTarget;
    bool m_renderDepth{true};
};

} // namespace huedra