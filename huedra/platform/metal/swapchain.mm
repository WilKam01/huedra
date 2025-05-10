#include "swapchain.hpp"
#include "core/log.hpp"
#include "graphics/graphics_manager.hpp"

namespace {

void fetchDrawableThread(bool& running, std::atomic_bool& alreadyAquiredDrawable, std::mutex& mutex,
                         std::condition_variable& condition, id<CAMetalDrawable>& drawable, CAMetalLayer*& layer)
{
    while (running)
    {
        drawable = [layer nextDrawable];
        alreadyAquiredDrawable.store(true);

        std::unique_lock<std::mutex> lock(mutex);
        condition.wait(lock,
                       [&alreadyAquiredDrawable, &running] { return !alreadyAquiredDrawable.load() || !running; });
    }
}

} // namespace

namespace huedra {

void MetalSwapchain::init(id<MTLDevice> device, id<MTLCommandQueue> commandQueue, WindowCocoa* window, bool renderDepth)
{
    m_device = device;
    m_commandQueue = commandQueue;
    m_window = window;
    m_renderDepth = renderDepth;
    m_currentDrawable = nil;
    m_presentSemaphore = dispatch_semaphore_create(1);
    m_textureRendered = false;

    m_layer = [CAMetalLayer layer];
    m_layer.device = m_device;
    m_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    m_layer.framebufferOnly = NO;
    m_layer.contentsScale = m_window->getScreenDPI();
    m_layer.frame = m_window->get().contentView.bounds;
    m_layer.maximumDrawableCount = GraphicsManager::MAX_FRAMES_IN_FLIGHT;
    m_layer.presentsWithTransaction = NO;
    [m_window->get().contentView setLayer:m_layer];
    [m_window->get().contentView setWantsLayer:YES];

    m_renderTarget.init(m_device, RenderTargetType::COLOR, GraphicsDataFormat::RGBA_8_UNORM,
                        static_cast<u32>(m_window->getScreenSize().x * m_window->getScreenDPI()),
                        static_cast<u32>(m_window->getScreenSize().y * m_window->getScreenDPI()));
    m_window->setRenderTarget(Ref<RenderTarget>(&m_renderTarget));

    m_threadInfo.runningThread = true;
    m_threadInfo.alreadyFetchedDrawable = false;
    m_threadInfo.fetchDrawableThread =
        new std::thread(::fetchDrawableThread, std::ref(m_threadInfo.runningThread),
                        std::ref(m_threadInfo.alreadyFetchedDrawable), std::ref(m_threadInfo.conditionMutex),
                        std::ref(m_threadInfo.condition), std::ref(m_currentDrawable), std::ref(m_layer));
}

void MetalSwapchain::cleanup()
{
    [m_presentSemaphore release];

    m_threadInfo.runningThread = false;
    m_threadInfo.condition.notify_one();
    m_threadInfo.fetchDrawableThread->join();
    delete m_threadInfo.fetchDrawableThread;

    [m_layer release];
}

void MetalSwapchain::aquireNextDrawable()
{
    if (m_window->isMinimized())
    {
        m_renderTarget.setAvailability(false);
        return;
    }

    m_renderTarget.setAvailability(m_threadInfo.runningThread);

    // Until texture has been rendered, don't present
    if (m_threadInfo.alreadyFetchedDrawable.load() && m_textureRendered)
    {
        id<MTLCommandBuffer> cmd = [m_commandQueue commandBuffer];
        [cmd addCompletedHandler:^(id<MTLCommandBuffer> _) {
          m_threadInfo.condition.notify_one();
          dispatch_semaphore_signal(m_presentSemaphore);
        }];

        id<MTLBlitCommandEncoder> blitEncoder = [cmd blitCommandEncoder];
        id<MTLTexture> texture = m_renderTarget.getMetalColorTexture().getWithIndex(0);
        [blitEncoder copyFromTexture:texture
                         sourceSlice:0
                         sourceLevel:0
                        sourceOrigin:MTLOriginMake(0, 0, 0)
                          sourceSize:MTLSizeMake(texture.width, texture.height, 1)
                           toTexture:m_currentDrawable.texture
                    destinationSlice:0
                    destinationLevel:0
                   destinationOrigin:MTLOriginMake(0, 0, 0)];
        [blitEncoder endEncoding];

        [cmd presentDrawable:m_currentDrawable];
        [cmd commit];

        [blitEncoder release];
        [cmd release];

        m_threadInfo.alreadyFetchedDrawable.store(false);
    }

    if (m_window->getScreenSize() != m_renderTarget.getSize())
    {
        if (m_threadInfo.alreadyFetchedDrawable.load())
        {
            dispatch_semaphore_wait(m_presentSemaphore, DISPATCH_TIME_FOREVER);
        }
        m_layer.drawableSize = CGSizeMake(m_window->getScreenSize().x * m_window->getScreenDPI(),
                                          m_window->getScreenSize().y * m_window->getScreenDPI());
        m_renderTarget.recreate(static_cast<u32>(m_window->getScreenSize().x * m_window->getScreenDPI()),
                                static_cast<u32>(m_window->getScreenSize().y * m_window->getScreenDPI()));

        m_threadInfo.alreadyFetchedDrawable.store(false);
        m_threadInfo.condition.notify_one();
        m_textureRendered = false;
    }
}

void MetalSwapchain::releaseDrawable()
{
    [m_currentDrawable release];
    m_threadInfo.alreadyFetchedDrawable.store(false);
    m_threadInfo.condition.notify_one();
}

} // namespace huedra