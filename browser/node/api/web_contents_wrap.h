// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#ifndef BREACH_BROWSER_NODE_API_WEB_CONTENTS_WRAP_H_
#define BREACH_BROWSER_NODE_API_WEB_CONTENTS_WRAP_H_

#include "base/memory/scoped_ptr.h"
#include "v8/include/v8.h"
#include "breach/browser/node/api/object_wrap.h"

namespace node {
class ObjectWrap;
}

namespace content {
class WebContents;
}

namespace breach {

class ExoBrowserWrap;
class ExoFrameWrap;

// ## WebContentsWrap
//
// This is a nodeJS wrapper for WebContents objects. There is no guarantee that
// the underlying WebContents is still alive, so this wrapper should be use
// with extreme care.
//
// The WebContentsWrap objects are created from the ExoBrowserWrap callbacks
// methods (coming from the ExoBrowser WebContentsDelegate API) and are passed
// to the JS context. They are generally passed back to the ExoBrowserWrap code
// and finally an ExoFrame through the _addFrame method.
//
// No API is exposed to JS so that these wrappers are opaque objects from JS.
// The web_contents_ pointer is directly set and accessed from the 
// ExoBrowserWrap and ExoFrameWrap classes.
class WebContentsWrap : public ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> exports);

private:
  WebContentsWrap();
  ~WebContentsWrap();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  /****************************************************************************/
  /*                               MEMBERS                                    */
  /****************************************************************************/
  WebContents*   web_contents_;

  static v8::Persistent<v8::Function>  s_constructor;

  friend class base::RefCountedThreadSafe<WebContentsWrap>;
  friend class ExoBrowserWrap;
  friend class ExoFrameWrap;

  DISALLOW_COPY_AND_ASSIGN(WebContentsWrap);
};

} // namespace breach

#endif // BREACH_BROWSER_NODE_API_WEB_CONTENTS_WRAP_H_

