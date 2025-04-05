#import "window.hpp"
#import "core/log.hpp"
#include "core/types.hpp"
#include "window/window.hpp"
#include <CoreFoundation/CFCGTypes.h>
#include <Foundation/Foundation.h>

#include <AppKit/NSEvent.h>
#import <Cocoa/Cocoa.h>

static huedra::WindowCocoa* cppWindow{nullptr};

@interface MyWindowDelegate : NSObject <NSWindowDelegate>
@end

@implementation MyWindowDelegate

- (void)windowDidMove:(NSNotification*)notification
{
    using namespace huedra;
    NSWindow* window = notification.object;
    NSRect frame = [window frame];
    NSRect contentFrame = [window contentRectForFrameRect:frame];
    NSRect mainFrame = [NSScreen mainScreen].frame;

    i32 xPos(static_cast<i32>(frame.origin.x));
    i32 yPos(static_cast<i32>(mainFrame.size.height - frame.origin.y - frame.size.height));
    i32 screenXPos(static_cast<i32>(contentFrame.origin.x));
    i32 screenYPos(static_cast<i32>(mainFrame.size.height - contentFrame.origin.y - contentFrame.size.height));
    cppWindow->updatePositionInternal(xPos, yPos, screenXPos, screenYPos);
}
- (void)windowDidResize:(NSNotification*)notification
{
    using namespace huedra;
    NSWindow* window = notification.object;
    NSRect frame = [window frame];
    NSRect contentFrame = [window contentRectForFrameRect:frame];

    u32 width{static_cast<u32>(frame.size.width)};
    u32 height{static_cast<u32>(frame.size.height)};
    u32 screenWidth{static_cast<u32>(contentFrame.size.width)};
    u32 screenHeight{static_cast<u32>(contentFrame.size.height)};
    cppWindow->updateResolutionInternal(width, height, screenWidth, screenHeight);
}

- (void)windowWillClose:(NSNotification*)notification
{
    cppWindow->setShouldClose();
}

@end

namespace huedra {

struct WindowCocoa::Impl
{
    NSWindow* window;
};

bool WindowCocoa::init(const std::string& title, const WindowInput& input)
{
    m_impl = new Impl();
    NSRect mainFrame = [NSScreen mainScreen].frame;

    i32 xPos = input.xPos.value_or(0);
    i32 yPos = static_cast<i32>(mainFrame.size.height) - input.yPos.value_or(0) + static_cast<i32>(input.height);

    NSRect frame = NSMakeRect(xPos, yPos, input.width, input.height);
    m_impl->window =
        [[NSWindow alloc] initWithContentRect:frame
                                    styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                                               NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable)
                                      backing:NSBackingStoreBuffered
                                        defer:NO];
    [m_impl->window
        setCollectionBehavior:(NSWindowCollectionBehaviorManaged | NSWindowCollectionBehaviorFullScreenAllowsTiling)];

    [m_impl->window setTitle:[NSString stringWithUTF8String:title.c_str()]];
    [m_impl->window makeKeyAndOrderFront:nil];
    if (!input.xPos.has_value() || !input.yPos.has_value())
    {
        [m_impl->window center];
    }

    MyWindowDelegate* delegate = [[MyWindowDelegate alloc] init];
    cppWindow = this;
    [m_impl->window setDelegate:delegate];

    // Init base window class with position and resolution
    WindowRect rect{};
    frame = [m_impl->window frame];
    NSRect contentFrame = [m_impl->window contentRectForFrameRect:frame];

    rect.xPos = static_cast<i32>(frame.origin.x);
    rect.yPos = static_cast<i32>(mainFrame.size.height - frame.origin.y - frame.size.height);
    rect.screenXPos = static_cast<i32>(contentFrame.origin.x);
    rect.screenYPos = static_cast<i32>(mainFrame.size.height - contentFrame.origin.y - contentFrame.size.height);
    rect.width = static_cast<u32>(frame.size.width);
    rect.height = static_cast<u32>(frame.size.height);
    rect.screenWidth = static_cast<u32>(contentFrame.size.width);
    rect.screenHeight = static_cast<u32>(contentFrame.size.height);
    Window::init(title, rect);

    log(LogLevel::D_INFO, "Created window!");
    return true;
}

void WindowCocoa::cleanup()
{
    [m_impl->window close];
    delete m_impl;
}

bool WindowCocoa::update()
{
    NSEvent* event{nullptr};
    while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES]))
    {
        switch ([event type])
        {
        default:
            break;
        }
        [NSApp sendEvent:event];
    }
    return !m_shouldClose;
}

void WindowCocoa::setTitle(const std::string& title)
{
    [m_impl->window setTitle:[NSString stringWithUTF8String:title.c_str()]];
    updateTitle(title);
}

void WindowCocoa::setResolution(u32 width, u32 height)
{
    NSRect frame = [m_impl->window frame];
    NSRect mainFrame = [NSScreen mainScreen].frame;

    WindowRect rect = getRect();
    [m_impl->window setFrame:NSMakeRect(rect.xPos, mainFrame.size.height - rect.yPos - frame.size.height, width, height)
                     display:YES
                     animate:YES];
}

void WindowCocoa::setPosition(i32 x, i32 y)
{
    NSRect frame = [m_impl->window frame];
    NSRect mainFrame = [NSScreen mainScreen].frame;

    WindowRect rect = getRect();
    [m_impl->window setFrame:NSMakeRect(x, mainFrame.size.height - y - frame.size.height, rect.width, rect.height)
                     display:YES
                     animate:YES];
}

void WindowCocoa::updatePositionInternal(i32 xPos, i32 yPos, i32 screenXPos, i32 screenYPos)
{
    updatePosition(xPos, yPos, screenXPos, screenYPos);
}

void WindowCocoa::updateResolutionInternal(u32 width, u32 height, u32 screenWidth, u32 screenHeight)
{
    updateResolution(width, height, screenWidth, screenHeight);
}

double WindowCocoa::getScreenDPI() const { return [m_impl->window backingScaleFactor]; }

} // namespace huedra