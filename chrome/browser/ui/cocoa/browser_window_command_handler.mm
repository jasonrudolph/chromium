// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "chrome/browser/ui/cocoa/browser_window_command_handler.h"

#include "base/logging.h"
#import "base/mac/foundation_util.h"
#include "base/strings/sys_string_conversions.h"
#include "chrome/app/chrome_command_ids.h"
#import "chrome/browser/app_controller_mac.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser_commands.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_window.h"
#include "content/public/browser/web_contents.h"
#import "ui/base/cocoa/cocoa_base_utils.h"
#include "ui/views_bridge_mac/bridged_native_widget_impl.h"
#include "ui/views_bridge_mac/mojo/bridged_native_widget_host.mojom.h"

namespace {

void SetToggleState(bool toggled, id item) {
  NSMenuItem* menuItem = base::mac::ObjCCast<NSMenuItem>(item);
  NSButton* buttonItem = base::mac::ObjCCast<NSButton>(item);
  if (menuItem) {
    NSInteger old_state = [menuItem state];
    NSInteger new_state = toggled ? NSOnState : NSOffState;
    if (old_state != new_state)
      [menuItem setState:new_state];
  } else if (buttonItem) {
    NSInteger old_state = [buttonItem state];
    NSInteger new_state = toggled ? NSOnState : NSOffState;
    if (old_state != new_state)
      [buttonItem setState:new_state];
  }
}

// Identify the actual Browser to which the command should be dispatched. It
// might belong to a background window, yet another dispatcher gets it because
// it is the foreground window's dispatcher and thus in the responder chain.
// Some senders don't have this problem (for example, menus only operate on the
// foreground window), so this is only an issue for senders that are part of
// windows.
Browser* FindBrowserForSender(id sender, NSWindow* window) {
  NSWindow* targetWindow = window;
  if ([sender respondsToSelector:@selector(window)])
    targetWindow = [sender window];
  Browser* browser = chrome::FindBrowserWithWindow(targetWindow);
  DCHECK(browser);
  return browser;
}

}  // namespace

@implementation BrowserWindowCommandHandler

- (BOOL)validateUserInterfaceItem:(id<NSValidatedUserInterfaceItem>)item
                           window:(NSWindow*)window {
  if ([item action] != @selector(commandDispatch:) &&
      [item action] != @selector(commandDispatchUsingKeyModifiers:)) {
    // NSWindow should only forward the above selectors here. All other
    // selectors must be handled by the default -[NSWindow
    // validateUserInterfaceItem:window:].
    NOTREACHED();
    // By default, interface items are enabled if the object in the responder
    // chain that implements the action does not implement
    // -validateUserInterfaceItem. Since we only care about -commandDispatch,
    // return YES for all other actions.
    return YES;
  }

  auto* bridge = views::BridgedNativeWidgetImpl::GetFromNativeWindow(window);
  DCHECK(bridge);

  views_bridge_mac::mojom::ValidateUserInterfaceItemResultPtr result;
  if (!bridge->host()->ValidateUserInterfaceItem([item tag], &result))
    return NO;

  if (result->set_toggle_state)
    SetToggleState(result->new_toggle_state, item);

  if (NSMenuItem* menuItem = base::mac::ObjCCast<NSMenuItem>(item)) {
    if (result->disable_if_has_no_key_equivalent)
      result->enable &= !![[menuItem keyEquivalent] length];

    if (result->set_hidden_state)
      [menuItem setHidden:result->new_hidden_state];

    if (result->new_title)
      [menuItem setTitle:base::SysUTF16ToNSString(*result->new_title)];
  }

  return result->enable;
}

- (void)commandDispatch:(id)sender window:(NSWindow*)window {
  DCHECK(sender);
  int command = [sender tag];
  chrome::ExecuteCommand(FindBrowserForSender(sender, window), command);
}

- (void)commandDispatchUsingKeyModifiers:(id)sender window:(NSWindow*)window {
  DCHECK(sender);

  if (![sender isEnabled]) {
    // This code is reachable e.g. if the user mashes the back button, queuing
    // up a bunch of events before the button's enabled state is updated:
    // http://crbug.com/63254
    return;
  }

  NSInteger command = [sender tag];
  NSUInteger modifierFlags = [[NSApp currentEvent] modifierFlags];
  if ((command == IDC_RELOAD) &&
      (modifierFlags & (NSShiftKeyMask | NSControlKeyMask))) {
    command = IDC_RELOAD_BYPASSING_CACHE;
    // Mask off Shift and Control so they don't affect the disposition below.
    modifierFlags &= ~(NSShiftKeyMask | NSControlKeyMask);
  }
  if (![[sender window] isMainWindow]) {
    // Remove the command key from the flags, it means "keep the window in
    // the background" in this case.
    modifierFlags &= ~NSCommandKeyMask;
  }
  chrome::ExecuteCommandWithDisposition(
      FindBrowserForSender(sender, window), command,
      ui::WindowOpenDispositionFromNSEventWithFlags([NSApp currentEvent],
                                                    modifierFlags));
}

@end
