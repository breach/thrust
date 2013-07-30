// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "breach/browser/browser.h"

#include "base/command_line.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "ui/aura/env.h"
#include "ui/aura/root_window.h"
#include "ui/aura/window.h"
#include "ui/base/accessibility/accessibility_types.h"
#include "ui/base/clipboard/clipboard.h"
#include "ui/base/events/event.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/screen.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/controls/textfield/textfield_controller.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/test/desktop_test_views_delegate.h"
#include "ui/views/view.h"
#include "ui/views/widget/desktop_aura/desktop_screen.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"

namespace breach {

namespace {
// ViewDelegate implementation for aura Breach
class BreachViewsDelegateAura : public views::DesktopTestViewsDelegate {
 public:
  BreachViewsDelegateAura() 
    : use_transparent_windows_(false) 
  {
  }

  virtual 
  ~BreachViewsDelegateAura() 
  {
  }

  void 
  SetUseTransparentWindows(
      bool transparent) 
  {
    use_transparent_windows_ = transparent;
  }

  // Overridden from views::TestViewsDelegate:
  virtual bool 
  UseTransparentWindows() const OVERRIDE {
    return use_transparent_windows_;
  }

 private:
  bool use_transparent_windows_;

  DISALLOW_COPY_AND_ASSIGN(BreachViewsDelegateAura);
};

// Maintain the UI controls and web view for Breach
class BreachWindowDelegateView : public views::WidgetDelegateView,
                                 public views::TextfieldController,
                                 public views::ButtonListener {
 public:
  enum UIControl {
    BACK_BUTTON,
    FORWARD_BUTTON,
    STOP_BUTTON
  };

  BreachWindowDelegateView(Browser* browser)
    : browser_(browser),
      toolbar_view_(new View),
      contents_view_(new View) 
  {
  }

  virtual 
  ~BreachWindowDelegateView() 
  {
  }

  // Update the state of UI controls
  void 
  SetAddressBarURL(
      const GURL& url) 
  {
    url_entry_->SetText(ASCIIToUTF16(url.spec()));
  }

  void 
  SetWebContents(
      WebContents* web_contents) 
  {
    contents_view_->SetLayoutManager(new views::FillLayout());
    web_view_ = new views::WebView(web_contents->GetBrowserContext());
    web_view_->SetWebContents(web_contents);
    web_contents->GetView()->Focus();
    contents_view_->AddChildView(web_view_);
    Layout();
  }
  void SetWindowTitle(const string16& title) { title_ = title; }
  void EnableUIControl(UIControl control, bool is_enabled) {
    if (control == BACK_BUTTON) {
      back_button_->SetState(is_enabled ? views::CustomButton::STATE_NORMAL
          : views::CustomButton::STATE_DISABLED);
    } else if (control == FORWARD_BUTTON) {
      forward_button_->SetState(is_enabled ? views::CustomButton::STATE_NORMAL
          : views::CustomButton::STATE_DISABLED);
    } else if (control == STOP_BUTTON) {
      stop_button_->SetState(is_enabled ? views::CustomButton::STATE_NORMAL
          : views::CustomButton::STATE_DISABLED);
    }
  }

 private:
  // Initialize the UI control contained in the Browser window
  void 
  InitBrowserWindow() 
  {
    set_background(views::Background::CreateStandardPanelBackground());

    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet* column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, 2);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1,
                          views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, 2);

    layout->AddPaddingRow(0, 2);

    // Add toolbar buttons and URL text field
    {
      layout->StartRow(0, 0);
      views::GridLayout* toolbar_layout = new views::GridLayout(toolbar_view_);
      toolbar_view_->SetLayoutManager(toolbar_layout);

      views::ColumnSet* toolbar_column_set =
          toolbar_layout->AddColumnSet(0);
      // Back button
      back_button_ = new views::LabelButton(this, ASCIIToUTF16("Back"));
      back_button_->SetStyle(views::Button::STYLE_NATIVE_TEXTBUTTON);
      gfx::Size back_button_size = back_button_->GetPreferredSize();
      toolbar_column_set->AddColumn(views::GridLayout::CENTER,
                                    views::GridLayout::CENTER, 0,
                                    views::GridLayout::FIXED,
                                    back_button_size.width(),
                                    back_button_size.width() / 2);
      // Forward button
      forward_button_ = new views::LabelButton(this, ASCIIToUTF16("Forward"));
      forward_button_->SetStyle(views::Button::STYLE_NATIVE_TEXTBUTTON);
      gfx::Size forward_button_size = forward_button_->GetPreferredSize();
      toolbar_column_set->AddColumn(views::GridLayout::CENTER,
                                    views::GridLayout::CENTER, 0,
                                    views::GridLayout::FIXED,
                                    forward_button_size.width(),
                                    forward_button_size.width() / 2);
      // Refresh button
      refresh_button_ = new views::LabelButton(this, ASCIIToUTF16("Refresh"));
      refresh_button_->SetStyle(views::Button::STYLE_NATIVE_TEXTBUTTON);
      gfx::Size refresh_button_size = refresh_button_->GetPreferredSize();
      toolbar_column_set->AddColumn(views::GridLayout::CENTER,
                                    views::GridLayout::CENTER, 0,
                                    views::GridLayout::FIXED,
                                    refresh_button_size.width(),
                                    refresh_button_size.width() / 2);
      // Stop button
      stop_button_ = new views::LabelButton(this, ASCIIToUTF16("Stop"));
      stop_button_->SetStyle(views::Button::STYLE_NATIVE_TEXTBUTTON);
      gfx::Size stop_button_size = stop_button_->GetPreferredSize();
      toolbar_column_set->AddColumn(views::GridLayout::CENTER,
                                    views::GridLayout::CENTER, 0,
                                    views::GridLayout::FIXED,
                                    stop_button_size.width(),
                                    stop_button_size.width() / 2);
      toolbar_column_set->AddPaddingColumn(0, 2);
      // URL entry
      url_entry_ = new views::Textfield();
      url_entry_->SetController(this);
      toolbar_column_set->AddColumn(views::GridLayout::FILL,
                                    views::GridLayout::FILL, 1,
                                    views::GridLayout::USE_PREF, 0, 0);

      // Fill up the first row
      toolbar_layout->StartRow(0, 0);
      toolbar_layout->AddView(back_button_);
      toolbar_layout->AddView(forward_button_);
      toolbar_layout->AddView(refresh_button_);
      toolbar_layout->AddView(stop_button_);
      toolbar_layout->AddView(url_entry_);

      layout->AddView(toolbar_view_);
    }

    layout->AddPaddingRow(0, 5);

    // Add web contents view as the second row
    {
      layout->StartRow(1, 0);
      layout->AddView(contents_view_);
    }

    layout->AddPaddingRow(0, 5);
  }
  // Overridden from TextfieldController
  virtual void ContentsChanged(views::Textfield* sender,
                               const string16& new_contents) OVERRIDE {
  }
  virtual bool HandleKeyEvent(views::Textfield* sender,
                              const ui::KeyEvent& key_event) OVERRIDE {
   if (sender == url_entry_ && key_event.key_code() == ui::VKEY_RETURN) {
     std::string text = UTF16ToUTF8(url_entry_->text());
     GURL url(text);
     if (!url.has_scheme()) {
       url = GURL(std::string("http://") + std::string(text));
       url_entry_->SetText(ASCIIToUTF16(url.spec()));
     }
     browser_->LoadURL(url);
     return true;
   }
   return false;
  }

  // Overridden from ButtonListener
  virtual void ButtonPressed(views::Button* sender,
                             const ui::Event& event) OVERRIDE {
    if (sender == back_button_)
      browser_->GoBackOrForward(-1);
    else if (sender == forward_button_)
      browser_->GoBackOrForward(1);
    else if (sender == refresh_button_)
      browser_->Reload();
    else if (sender == stop_button_)
      browser_->Stop();
  }

  // Overridden from WidgetDelegateView
  virtual bool CanResize() const OVERRIDE { return true; }
  virtual bool CanMaximize() const OVERRIDE { return true; }
  virtual string16 GetWindowTitle() const OVERRIDE {
    return title_;
  }
  virtual void WindowClosing() OVERRIDE {
    if (browser_) {
      delete browser_;
      browser_ = NULL;
    }
  }
  virtual View* GetContentsView() OVERRIDE { return this; }

  // Overridden from View
  virtual void ViewHierarchyChanged(
      const ViewHierarchyChangedDetails& details) OVERRIDE {
    if (details.is_add && details.child == this) {
      InitBrowserWindow();
    }
  }

 private:
  // Hold a reference of Browser for deleting it when the window is closing
  Browser* browser_;

  // Window title
  string16 title_;

  // Toolbar view contains forward/backward/reload button and URL entry
  View* toolbar_view_;
  views::LabelButton* back_button_;
  views::LabelButton* forward_button_;
  views::LabelButton* refresh_button_;
  views::LabelButton* stop_button_;
  views::Textfield* url_entry_;

  // Contents view contains the web contents view
  View* contents_view_;
  views::WebView* web_view_;

  DISALLOW_COPY_AND_ASSIGN(BreachWindowDelegateView);
};

}  // namespace

views::ViewsDelegate* Browser::views_delegate_ = NULL;

// static
void 
Browser::PlatformInitialize(
    const gfx::Size& default_window_size) 
{
  gfx::Screen::SetScreenInstance(
      gfx::SCREEN_TYPE_NATIVE, views::CreateDesktopScreen());
  views_delegate_ = new BreachViewsDelegateAura();
}

void 
Browser::PlatformExit() 
{
  if (views_delegate_)
    delete views_delegate_;
  aura::Env::DeleteInstance();
}

void 
Browser::PlatformCleanUp() 
{
}

void 
Browser::PlatformEnableUIControl(
    UIControl control, 
    bool is_enabled) 
{
  BreachWindowDelegateView* delegate_view =
    static_cast<BreachWindowDelegateView*>(window_widget_->widget_delegate());
  if (control == BACK_BUTTON) {
    delegate_view->EnableUIControl(BreachWindowDelegateView::BACK_BUTTON,
        is_enabled);
  } else if (control == FORWARD_BUTTON) {
    delegate_view->EnableUIControl(BreachWindowDelegateView::FORWARD_BUTTON,
        is_enabled);
  } else if (control == STOP_BUTTON) {
    delegate_view->EnableUIControl(BreachWindowDelegateView::STOP_BUTTON,
        is_enabled);
  }
}

void 
Browser::PlatformSetAddressBarURL(
    const GURL& url) 
{
  BreachWindowDelegateView* delegate_view =
    static_cast<BreachWindowDelegateView*>(window_widget_->widget_delegate());
  delegate_view->SetAddressBarURL(url);
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
  window_widget_ =
      views::Widget::CreateWindowWithBounds(new BreachWindowDelegateView(this),
               gfx::Rect(0, 0, width, height));

  window_ = window_widget_->GetNativeWindow();
  window_->GetRootWindow()->ShowRootWindow();
  window_widget_->Show();
}

void 
Browser::PlatformSetContents() 
{
  BreachWindowDelegateView* delegate_view =
    static_cast<BreachWindowDelegateView*>(window_widget_->widget_delegate());
  delegate_view->SetWebContents(web_contents_.get());
}

void 
Browser::PlatformResizeSubViews() 
{
}

void 
Browser::Close() 
{
  window_widget_->Close();
}

void 
Browser::PlatformSetTitle(const string16& title) 
{
  BreachWindowDelegateView* delegate_view =
    static_cast<BreachWindowDelegateView*>(window_widget_->widget_delegate());
  delegate_view->SetWindowTitle(title);
  window_widget_->UpdateWindowTitle();
}

}  // namespace content
