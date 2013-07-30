// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "breach/browser/ui/browser.h"

#include <commctrl.h>
#include <fcntl.h>
#include <io.h>
#include <windows.h>

#include "base/strings/utf_string_conversions.h"
#include "base/win/wrapped_window_proc.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "breach/app/resource.h"
#include "ui/base/win/hwnd_util.h"

using namespace content;

namespace {

const wchar_t kWindowTitle[] = L"Breach";
const wchar_t kWindowClass[] = L"BREACH";

const int kButtonWidth = 72;
const int kURLBarHeight = 24;

const int kMaxURLLength = 1024;

}  // namespace

namespace breach {

HINSTANCE Browser::instance_handle_;

void 
Browser::PlatformInitialize(
    const gfx::Size& default_window_size) 
{
  _setmode(_fileno(stdout), _O_BINARY);
  _setmode(_fileno(stderr), _O_BINARY);
  INITCOMMONCONTROLSEX InitCtrlEx;
  InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
  InitCtrlEx.dwICC  = ICC_STANDARD_CLASSES;
  InitCommonControlsEx(&InitCtrlEx);
  RegisterWindowClass();
}

void 
Browser::PlatformExit() 
{
  std::vector<Browser*> windows = windows_;
  for (std::vector<Browser*>::iterator it = windows.begin();
       it != windows.end(); ++it)
    DestroyWindow((*it)->window_);
}

void 
Browser::PlatformCleanUp() 
{
  // When the window is destroyed, tell the Edit field to forget about us,
  // otherwise we will crash.
  ui::SetWindowProc(url_edit_view_, default_edit_wnd_proc_);
  ui::SetWindowUserData(url_edit_view_, NULL);
}

void 
Browser::PlatformEnableUIControl(
    UIControl control, 
    bool is_enabled) 
{
  int id;
  switch (control) {
    case BACK_BUTTON:
      id = IDC_NAV_BACK;
      break;
    case FORWARD_BUTTON:
      id = IDC_NAV_FORWARD;
      break;
    case STOP_BUTTON:
      id = IDC_NAV_STOP;
      break;
    default:
      NOTREACHED() << "Unknown UI control";
      return;
  }
  EnableWindow(GetDlgItem(window_, id), is_enabled);
}

void 
Browser::PlatformSetAddressBarURL(
    const GURL& url) 
{
  std::wstring url_string = UTF8ToWide(url.spec());
  SendMessage(url_edit_view_, WM_SETTEXT, 0,
              reinterpret_cast<LPARAM>(url_string.c_str()));
}

void 
Browser::PlatformSetIsLoading(
    bool loading) 
{
}

void 
Browser::PlatformCreateWindow(
    int width, 
    int height) 
{
  window_ = CreateWindow(kWindowClass, kWindowTitle,
                         WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                         CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
                         NULL, NULL, instance_handle_, NULL);
  ui::SetWindowUserData(window_, this);

  HWND hwnd;
  int x = 0;

  hwnd = CreateWindow(L"BUTTON", L"Back",
                      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON ,
                      x, 0, kButtonWidth, kURLBarHeight,
                      window_, (HMENU) IDC_NAV_BACK, instance_handle_, 0);
  x += kButtonWidth;

  hwnd = CreateWindow(L"BUTTON", L"Forward",
                      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON ,
                      x, 0, kButtonWidth, kURLBarHeight,
                      window_, (HMENU) IDC_NAV_FORWARD, instance_handle_, 0);
  x += kButtonWidth;

  hwnd = CreateWindow(L"BUTTON", L"Reload",
                      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON ,
                      x, 0, kButtonWidth, kURLBarHeight,
                      window_, (HMENU) IDC_NAV_RELOAD, instance_handle_, 0);
  x += kButtonWidth;

  hwnd = CreateWindow(L"BUTTON", L"Stop",
                      WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON ,
                      x, 0, kButtonWidth, kURLBarHeight,
                      window_, (HMENU) IDC_NAV_STOP, instance_handle_, 0);
  x += kButtonWidth;

  // This control is positioned by PlatformResizeSubViews.
  url_edit_view_ = CreateWindow(L"EDIT", 0,
                                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT |
                                ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                                x, 0, 0, 0, window_, 0, instance_handle_, 0);

  default_edit_wnd_proc_ = ui::SetWindowProc(url_edit_view_,
                                             Browser::EditWndProc);
  ui::SetWindowUserData(url_edit_view_, this);

  ShowWindow(window_, SW_SHOW);

  SizeTo(width, height);
}

void 
Browser::PlatformSetContents() 
{
  SetParent(web_contents_->GetView()->GetNativeView(), window_);
}

void 
Browser::SizeTo(int width, int height) 
{
  RECT rc, rw;
  GetClientRect(window_, &rc);
  GetWindowRect(window_, &rw);

  int client_width = rc.right - rc.left;
  int window_width = rw.right - rw.left;
  window_width = (window_width - client_width) + width;

  int client_height = rc.bottom - rc.top;
  int window_height = rw.bottom - rw.top;
  window_height = (window_height - client_height) + height;

  // Add space for the url bar.
  window_height += kURLBarHeight;

  SetWindowPos(window_, NULL, 0, 0, window_width, window_height,
               SWP_NOMOVE | SWP_NOZORDER);
}

void 
Browser::PlatformResizeSubViews() 
{
  RECT rc;
  GetClientRect(window_, &rc);

  int x = kButtonWidth * 4;
  MoveWindow(url_edit_view_, x, 0, rc.right - x, kURLBarHeight, TRUE);

  MoveWindow(GetContentView(), 0, kURLBarHeight, rc.right,
             rc.bottom - kURLBarHeight, TRUE);
}

void 
Browser::Close() 
{
  DestroyWindow(window_);
}

ATOM 
Browser::RegisterWindowClass() 
{
  WNDCLASSEX window_class;
  base::win::InitializeWindowClass(
      kWindowClass,
      &Browser::WndProc,
      CS_HREDRAW | CS_VREDRAW,
      0,
      0,
      LoadCursor(NULL, IDC_ARROW),
      NULL,
      MAKEINTRESOURCE(IDC_CONTENTSHELL),
      NULL,
      NULL,
      &window_class);
  instance_handle_ = window_class.hInstance;
  return RegisterClassEx(&window_class);
}

LRESULT CALLBACK 
Browser::WndProc(
    HWND hwnd, 
    UINT message, 
    WPARAM wParam,
    LPARAM lParam) 
{
  Browser* browser = static_cast<Browser*>(ui::GetWindowUserData(hwnd));

  switch (message) {
    case WM_COMMAND: {
      int id = LOWORD(wParam);
      switch (id) {
        case IDM_NEW_WINDOW:
          CreateNewWindow(
              browser->web_contents()->GetBrowserContext(),
              GURL(), NULL, MSG_ROUTING_NONE, gfx::Size());
          break;
        case IDM_CLOSE_WINDOW:
          DestroyWindow(hwnd);
          break;
        case IDM_EXIT:
          PlatformExit();
          break;
        case IDM_SHOW_DEVELOPER_TOOLS:
          browser->ShowDevTools();
          break;
        case IDC_NAV_BACK:
          browser->GoBackOrForward(-1);
          break;
        case IDC_NAV_FORWARD:
          browser->GoBackOrForward(1);
          break;
        case IDC_NAV_RELOAD:
          browser->Reload();
          break;
        case IDC_NAV_STOP:
          browser->Stop();
          break;
      }
      break;
    }
    case WM_DESTROY: {
      delete browser;
      return 0;
    }

    case WM_SIZE: {
      if (browser->GetContentView())
        browser->PlatformResizeSubViews();
      return 0;
    }

    case WM_WINDOWPOSCHANGED: {
      // Notify the content view that the window position of its parent window
      // has been changed by sending window message
      gfx::NativeView native_view = browser->GetContentView();
      if (native_view) {
        SendMessage(native_view, message, wParam, lParam);
      }
      break;
   }
  }

  return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK 
Browser::EditWndProc(
    HWND hwnd, 
    UINT message,
    WPARAM wParam, 
    LPARAM lParam) 
{
  Browser* browser = static_cast<Browser*>(ui::GetWindowUserData(hwnd));

  switch (message) {
    case WM_CHAR:
      if (wParam == VK_RETURN) {
        wchar_t str[kMaxURLLength + 1];  // Leave room for adding a NULL;
        *(str) = kMaxURLLength;
        LRESULT str_len = SendMessage(hwnd, EM_GETLINE, 0, (LPARAM)str);
        if (str_len > 0) {
          str[str_len] = 0;  // EM_GETLINE doesn't NULL terminate.
          GURL url(str);
          if (!url.has_scheme())
            url = GURL(std::wstring(L"http://") + std::wstring(str));
          browser->LoadURL(url);
        }

        return 0;
      }
  }

  return CallWindowProc(browser->default_edit_wnd_proc_, hwnd, message, wParam,
                        lParam);
}

void 
Browser::PlatformSetTitle(
    const string16& text) 
{
  ::SetWindowText(window_, text.c_str());
}

} // namespace breach
