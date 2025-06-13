#include "window.hpp"
#include "core/global.hpp"
#include "core/input/keys.hpp"
#include "core/input/mouse.hpp"
#include "core/log.hpp"
#include "core/types.hpp"
#include "window/window.hpp"

#include <AppKit/NSEvent.h>
#include <CoreFoundation/CFCGTypes.h>
#include <Foundation/Foundation.h>

namespace {

NSRect findMainFrame()
{
    @autoreleasepool
    {
        for (NSScreen* screen in [NSScreen screens])
        {
            NSDictionary* desc = [screen deviceDescription];
            NSNumber* isPrimary = desc[@"NSScreenNumber"];

            if (NSMinX([screen frame]) == 0 && NSMinY([screen frame]) == 0)
            {
                return screen.frame;
            }
        }
    }
    return [NSScreen mainScreen].frame;
}

} // namespace

@interface MyWindowDelegate : NSObject <NSWindowDelegate>
@property(nonatomic, assign) huedra::WindowCocoa* cppWindow;
@end

@implementation MyWindowDelegate

- (void)windowDidMove:(NSNotification*)notification
{
    @autoreleasepool
    {
        using namespace huedra;
        NSWindow* window = notification.object;
        NSRect frame = [window frame];
        NSRect contentFrame = [window contentRectForFrameRect:frame];
        NSRect mainFrame = ::findMainFrame();

        i32 positionX(static_cast<i32>(frame.origin.x));
        i32 positionY(static_cast<i32>(mainFrame.size.height - frame.origin.y - frame.size.height));
        i32 screenPositionX(static_cast<i32>(contentFrame.origin.x));
        i32 screenPositionY(static_cast<i32>(mainFrame.size.height - contentFrame.origin.y - contentFrame.size.height));
        self.cppWindow->updatePositionInternal(positionX, positionY, screenPositionX, screenPositionY);

        u32 width{static_cast<u32>(frame.size.width)};
        u32 height{static_cast<u32>(frame.size.height)};
        u32 screenWidth{static_cast<u32>(contentFrame.size.width)};
        u32 screenHeight{static_cast<u32>(contentFrame.size.height)};
        self.cppWindow->updateResolutionInternal(width, height, screenWidth, screenHeight);
    }
}
- (void)windowDidResize:(NSNotification*)notification
{
    @autoreleasepool
    {
        using namespace huedra;
        NSWindow* window = notification.object;
        NSRect frame = [window frame];
        NSRect contentFrame = [window contentRectForFrameRect:frame];
        NSRect mainFrame = ::findMainFrame();

        i32 positionX(static_cast<i32>(frame.origin.x));
        i32 positionY(static_cast<i32>(mainFrame.size.height - frame.origin.y - frame.size.height));
        i32 screenPositionX(static_cast<i32>(contentFrame.origin.x));
        i32 screenPositionY(static_cast<i32>(mainFrame.size.height - contentFrame.origin.y - contentFrame.size.height));
        self.cppWindow->updatePositionInternal(positionX, positionY, screenPositionX, screenPositionY);

        u32 width{static_cast<u32>(frame.size.width)};
        u32 height{static_cast<u32>(frame.size.height)};
        u32 screenWidth{static_cast<u32>(contentFrame.size.width)};
        u32 screenHeight{static_cast<u32>(contentFrame.size.height)};
        self.cppWindow->updateResolutionInternal(width, height, screenWidth, screenHeight);
    }
}

- (void)windowDidBecomeKey:(NSNotification*)notification
{
    self.cppWindow->setFocusInternal(true);
}

- (void)windowDidResignKey:(NSNotification*)notification
{
    self.cppWindow->setFocusInternal(false);
}

- (void)windowWillClose:(NSNotification*)notification
{
    self.cppWindow->setShouldClose();
}

@end

namespace huedra {

bool WindowCocoa::init(const std::string& title, const WindowInput& input)
{
    @autoreleasepool
    {
        NSRect mainFrame = ::findMainFrame();

        i32 positionX = input.positionX.value_or(0);
        i32 positionY =
            static_cast<i32>(mainFrame.size.height) - input.positionY.value_or(0) + static_cast<i32>(input.height);

        NSRect frame = NSMakeRect(positionX, positionY, input.width, input.height);
        m_window = [[NSWindow alloc] initWithContentRect:frame
                                               styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                                                          NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable)
                                                 backing:NSBackingStoreBuffered
                                                   defer:NO];
        [m_window setCollectionBehavior:(NSWindowCollectionBehaviorManaged |
                                         NSWindowCollectionBehaviorFullScreenAllowsTiling)];

        [m_window setTitle:[NSString stringWithUTF8String:title.c_str()]];
        if (!input.positionX.has_value() || !input.positionY.has_value())
        {
            [m_window center];
        }

        MyWindowDelegate* delegate = [[MyWindowDelegate alloc] init];
        delegate.cppWindow = this;
        [m_window setDelegate:delegate];

        // Init base window class with position and resolution
        WindowRect rect{};
        frame = [m_window frame];
        NSRect contentFrame = [m_window contentRectForFrameRect:frame];

        rect.positionX = static_cast<i32>(frame.origin.x);
        rect.positionY = static_cast<i32>(mainFrame.size.height - frame.origin.y - frame.size.height);
        rect.screenPositionX = static_cast<i32>(contentFrame.origin.x);
        rect.screenPositionY =
            static_cast<i32>(mainFrame.size.height - contentFrame.origin.y - contentFrame.size.height);
        rect.width = static_cast<u32>(frame.size.width);
        rect.height = static_cast<u32>(frame.size.height);
        rect.screenWidth = static_cast<u32>(contentFrame.size.width);
        rect.screenHeight = static_cast<u32>(contentFrame.size.height);
        Window::init(title, rect);
    }

    return true;
}

void WindowCocoa::cleanup()
{
    Window::cleanup();
    [m_window close];
    [m_window release];
}

bool WindowCocoa::update()
{
    @autoreleasepool
    {
        NSEvent* event{nullptr};
        while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                           untilDate:nil
                                              inMode:NSDefaultRunLoopMode
                                             dequeue:YES]))
        {
            switch (event.type)
            {
            case NSEventTypeKeyDown:
                global::input.setKey(convertKey(event.keyCode, [event.characters characterAtIndex:0]), true);
                break;
            case NSEventTypeKeyUp:
                global::input.setKey(convertKey(event.keyCode, [event.characters characterAtIndex:0]), false);
                break;
            case NSEventTypeFlagsChanged: {
                u32 flags = event.modifierFlags;
                global::input.setKey(Keys::SHIFT, static_cast<bool>(flags & NSEventModifierFlagShift));
                global::input.setKey(Keys::CTRL, static_cast<bool>(flags & NSEventModifierFlagControl));
                global::input.setKey(Keys::OPTION, static_cast<bool>(flags & NSEventModifierFlagOption));
                global::input.setKey(Keys::CMD, static_cast<bool>(flags & NSEventModifierFlagCommand));
                global::input.setKey(Keys::CAPS_LOCK, static_cast<bool>(flags & NSEventModifierFlagCapsLock));
            }
            break;
            case NSEventTypeLeftMouseDown:
            case NSEventTypeRightMouseDown:
            case NSEventTypeOtherMouseDown: {
                MouseButton button{MouseButton::NONE};
                switch (event.buttonNumber)
                {
                case 0:
                    button = MouseButton::LEFT;
                    break;
                case 1:
                    button = MouseButton::RIGHT;
                    break;
                case 2:
                    button = MouseButton::MIDDLE;
                    break;
                case 3:
                    button = MouseButton::EXTRA1;
                    break;
                case 4:
                    button = MouseButton::EXTRA2;
                    break;
                default:
                    button = MouseButton::NONE;
                    break;
                }
                if (event.clickCount == 2)
                {
                    global::input.setMouseButtonDoubleClick(button);
                }
                global::input.setMouseButton(button, true);
            }
            break;
            case NSEventTypeLeftMouseUp:
            case NSEventTypeRightMouseUp:
            case NSEventTypeOtherMouseUp: {
                MouseButton button{MouseButton::NONE};
                switch (event.buttonNumber)
                {
                case 0:
                    button = MouseButton::LEFT;
                    break;
                case 1:
                    button = MouseButton::RIGHT;
                    break;
                case 2:
                    button = MouseButton::MIDDLE;
                    break;
                case 3:
                    button = MouseButton::EXTRA1;
                    break;
                case 4:
                    button = MouseButton::EXTRA2;
                    break;
                default:
                    button = MouseButton::NONE;
                    break;
                }
                global::input.setMouseButton(button, false);
            }
            break;
            case NSEventTypeScrollWheel:
                global::input.setMouseScrollHorizontal(static_cast<float>(event.scrollingDeltaX));
                global::input.setMouseScrollVertical(static_cast<float>(event.scrollingDeltaY));
                break;
            case NSEventTypeMouseMoved:
            case NSEventTypeLeftMouseDragged:
            case NSEventTypeRightMouseDragged:
            case NSEventTypeOtherMouseDragged: {
                NSRect mainFrame = ::findMainFrame();
                global::input.setMousePos(ivec2(static_cast<i32>([NSEvent mouseLocation].x),
                                                static_cast<i32>(mainFrame.size.height - [NSEvent mouseLocation].y)));
                global::input.setMouseDelta(ivec2(static_cast<i32>(event.deltaX), static_cast<i32>(event.deltaY)));
            }
            break;
            default:
                break;
            }
            // Prevent sounds on clicking of keyboard
            if (event.type != NSEventTypeKeyDown && event.type != NSEventTypeKeyUp)
            {
                [NSApp sendEvent:event];
            }
        }

        CGEventFlags flags = CGEventSourceFlagsState(kCGEventSourceStateHIDSystemState);
        global::input.setKeyToggle(KeyToggles::CAPS_LOCK, static_cast<bool>(flags & kCGEventFlagMaskAlphaShift));
        global::input.setKeyToggle(KeyToggles::NUM_LOCK, true);
        global::input.setKeyToggle(KeyToggles::SCR_LOCK, true);

        updateMinimized(static_cast<bool>(m_window.isMiniaturized));
    }
    return !m_shouldClose;
}

void WindowCocoa::setTitle(const std::string& title)
{
    @autoreleasepool
    {
        NSString* nsTitle = [NSString stringWithCString:title.c_str() encoding:NSUTF8StringEncoding];
        [m_window setTitle:nsTitle];
    }
    updateTitle(title);
}

void WindowCocoa::setResolution(u32 width, u32 height)
{
    @autoreleasepool
    {
        NSRect frame = [m_window frame];
        NSRect mainFrame = ::findMainFrame();

        WindowRect rect = getRect();
        [m_window setFrame:NSMakeRect(rect.positionX, mainFrame.size.height - rect.positionY - frame.size.height, width,
                                      height)
                   display:YES
                   animate:YES];
    }
}

void WindowCocoa::setPosition(i32 x, i32 y)
{
    @autoreleasepool
    {
        NSRect frame = [m_window frame];
        NSRect mainFrame = ::findMainFrame();

        WindowRect rect = getRect();
        [m_window setFrame:NSMakeRect(x, mainFrame.size.height - y - frame.size.height, rect.width, rect.height)
                   display:YES
                   animate:YES];
    }
}

void WindowCocoa::updatePositionInternal(i32 positionX, i32 positionY, i32 screenPositionX, i32 screenPositionY)
{
    updatePosition(positionX, positionY, screenPositionX, screenPositionY);
}

void WindowCocoa::updateResolutionInternal(u32 width, u32 height, u32 screenWidth, u32 screenHeight)
{
    updateResolution(width, height, screenWidth, screenHeight);
}

void WindowCocoa::setFocusInternal(bool isFocus)
{
    if (isFocus)
    {
        global::windowManager.setFocusedWindow(this);
    }
    else if (global::windowManager.getFocusedWindow().get() == this)
    {
        global::windowManager.setFocusedWindow(nullptr);
    }
}

double WindowCocoa::getScreenDPI() const { return [m_window backingScaleFactor]; }

Keys WindowCocoa::convertKey(u16 code, char character)
{
    character = static_cast<char>(toupper(character));
    if (character >= 'A' && character <= 'Z')
    {
        return static_cast<Keys>(static_cast<u32>(Keys::A) + static_cast<u32>(character - 'A'));
    }

    switch (code)
    {
    case 0x1D:
        return Keys::NUM_0;
    case 0x12:
        return Keys::NUM_1;
    case 0x13:
        return Keys::NUM_2;
    case 0x14:
        return Keys::NUM_3;
    case 0x15:
        return Keys::NUM_4;
    case 0x17:
        return Keys::NUM_5;
    case 0x16:
        return Keys::NUM_6;
    case 0x1a:
        return Keys::NUM_7;
    case 0x1c:
        return Keys::NUM_8;
    case 0x19:
        return Keys::NUM_9;
    case 0x7b:
        return Keys::ARR_LEFT;
    case 0x7c:
        return Keys::ARR_RIGHT;
    case 0x7e:
        return Keys::ARR_UP;
    case 0x7d:
        return Keys::ARR_DOWN;
    case 0x35:
        return Keys::ESCAPE;
    case 0x30:
        return Keys::TAB;
    case 0x33:
        return Keys::BACKSPACE;
    case 0x24:
        return Keys::ENTER;
    case 0x31:
        return Keys::SPACE;
    case 0x7a:
        return Keys::F1;
    case 0x78:
        return Keys::F2;
    case 0x63:
        return Keys::F3;
    case 0x76:
        return Keys::F4;
    case 0x60:
        return Keys::F5;
    case 0x61:
        return Keys::F6;
    case 0x62:
        return Keys::F7;
    case 0x64:
        return Keys::F8;
    case 0x65:
        return Keys::F9;
    case 0x6d:
        return Keys::F10;
    case 0x67:
        return Keys::F11;
    case 0x6F:
        return Keys::F12;
    case 0x2b:
        return Keys::COMMA;
    case 0x2f:
        return Keys::DOT;
    case 0x1b:
        return Keys::MINUS;
    case 0x18:
        return Keys::PLUS;
    case 0x72:
        return Keys::INSERT;
    case 0x75:
        return Keys::DEL;
    case 0x73:
        return Keys::HOME;
    case 0x77:
        return Keys::END;
    case 0x74:
        return Keys::PAGE_UP;
    case 0x79:
        return Keys::PAGE_DOWN;
    case 0x4b:
        return Keys::NUMPAD_DIVIDE;
    case 0x43:
        return Keys::NUMPAD_MULT;
    case 0x4e:
        return Keys::NUMPAD_MINUS;
    case 0x45:
        return Keys::NUMPAD_PLUS;
    case 0x41:
        return Keys::NUMPAD_DEL;
    case 0x52:
        return Keys::NUMPAD_0;
    case 0x53:
        return Keys::NUMPAD_1;
    case 0x54:
        return Keys::NUMPAD_2;
    case 0x55:
        return Keys::NUMPAD_3;
    case 0x56:
        return Keys::NUMPAD_4;
    case 0x57:
        return Keys::NUMPAD_5;
    case 0x58:
        return Keys::NUMPAD_6;
    case 0x59:
        return Keys::NUMPAD_7;
    case 0x5b:
        return Keys::NUMPAD_8;
    case 0x5c:
        return Keys::NUMPAD_9;
    default:
        return Keys::NONE;
    }
    return Keys::NONE;
}

} // namespace huedra