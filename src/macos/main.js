/* macOS system JavaScript for Automation + Cocoa/WebKit. No third-party runtime. */
ObjC.import('Cocoa');
ObjC.import('WebKit');
var app = $.NSApplication.sharedApplication;
app.setActivationPolicy($.NSApplicationActivationPolicyRegular);
var nativeWindow, webView;
ObjC.registerSubclass({
 name: 'FullscreenTimerDelegate',
 superclass: 'NSObject',
 protocols: ['NSWindowDelegate', 'WKScriptMessageHandler'],
 methods: {
  'userContentController:didReceiveScriptMessage:': {
   types: ['void', ['id', 'id']],
   implementation: function(controller, message) {
    if (ObjC.unwrap(message.body) === 'fullscreen') nativeWindow.toggleFullScreen(null);
   }
  },
  'windowDidExitFullScreen:': {
   types: ['void', ['id']],
   implementation: function(notification) {
    webView.evaluateJavaScriptCompletionHandler($("window.dispatchEvent(new Event('native-fullscreen-exit'))"), null);
   }
  },
  'windowDidEnterFullScreen:': {
   types: ['void', ['id']],
   implementation: function(notification) {
    webView.evaluateJavaScriptCompletionHandler($("window.dispatchEvent(new Event('native-fullscreen-enter'))"), null);
   }
  },
  'windowWillClose:': {
   types: ['void', ['id']],
   implementation: function(notification) { app.terminate(null); }
  }
 }
});
var delegate = $.FullscreenTimerDelegate.alloc.init;
function run(argv) {
if (argv[0] === '--self-test') {
 var probe = $.WKWebViewConfiguration.alloc.init;
 return probe ? 'PASS Cocoa/WebKit bridge available' : 'FAIL WebKit unavailable';
}
var htmlPath = argv[0];
if (!htmlPath) throw new Error('Missing local HTML path');
var frame = $.NSMakeRect(0, 0, 1280, 760);
nativeWindow = $.NSWindow.alloc.initWithContentRectStyleMaskBackingDefer(frame, 15, 2, false);
nativeWindow.title = $('Fullscreen Timer');
nativeWindow.minSize = $.NSMakeSize(480, 360);
nativeWindow.collectionBehavior = 128; // FullScreenPrimary
nativeWindow.delegate = delegate;
nativeWindow.backgroundColor = $.NSColor.blackColor;
nativeWindow.center;
var configuration = $.WKWebViewConfiguration.alloc.init;
configuration.userContentController.addScriptMessageHandlerName(delegate, $('native'));
webView = $.WKWebView.alloc.initWithFrameConfiguration(frame, configuration);
webView.autoresizingMask = 18;
nativeWindow.contentView = webView;
var url = $.NSURL.fileURLWithPath($(htmlPath));
webView.loadFileURLAllowingReadAccessToURL(url, url.URLByDeletingLastPathComponent);
var menuBar = $.NSMenu.alloc.init;
var appMenuItem = $.NSMenuItem.alloc.init;
var appMenu = $.NSMenu.alloc.init;
var quit = $.NSMenuItem.alloc.initWithTitleActionKeyEquivalent($('Quit Fullscreen Timer'), 'terminate:', $('q'));
appMenu.addItem(quit);
appMenuItem.submenu = appMenu;
menuBar.addItem(appMenuItem);
app.mainMenu = menuBar;
nativeWindow.makeKeyAndOrderFront(null);
app.activateIgnoringOtherApps(true);
app.run;
}
