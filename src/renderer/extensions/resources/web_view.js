// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

var DocumentNatives = requireNative('document_natives');
var WebViewNatives = requireNative('webview_natives');

/******************************************************************************/
/* ID GENERATOR */
/******************************************************************************/
var _id = 0;
var getNextId = function() {
  return _id++;
}

/******************************************************************************/
/* GLOBAL VALUES */
/******************************************************************************/
var ERROR_MSG_CONTENTWINDOW_NOT_AVAILABLE = '<webview>: ' +
  'contentWindow is not available at this time. It will become available ' +
  'when the page has finished loading.';

var WEB_VIEW_ATTRIBUTE_AUTOSIZE = 'autosize';
var WEB_VIEW_ATTRIBUTE_MAXHEIGHT = 'maxheight';
var WEB_VIEW_ATTRIBUTE_MAXWIDTH = 'maxwidth';
var WEB_VIEW_ATTRIBUTE_MINHEIGHT = 'minheight';
var WEB_VIEW_ATTRIBUTE_MINWIDTH = 'minwidth';
var AUTO_SIZE_ATTRIBUTES = [
  WEB_VIEW_ATTRIBUTE_AUTOSIZE,
  WEB_VIEW_ATTRIBUTE_MAXHEIGHT,
  WEB_VIEW_ATTRIBUTE_MAXWIDTH,
  WEB_VIEW_ATTRIBUTE_MINHEIGHT,
  WEB_VIEW_ATTRIBUTE_MINWIDTH,
];

var WEB_VIEW_ATTRIBUTES = [
    'allowtransparency',
];

/* TODO(spolu): FixMe Chrome 39 */
var PLUGIN_METHOD_ATTACH = '-internal-attach';

/******************************************************************************/
/* WEBVIEW INTERNAL */
/******************************************************************************/


// ## webview
var webview = function(spec, my) {
  my = my || {};
  spec = spec || {};

  my.webview_node = spec.node;
  my.browser_plugin_node = null;
  my.src_observer = null;

  my.src = null;
  my.attached = false;
  my.element_attached = false;
  my.deferred_attach = null;

  my.before_first_navigation = true;
  my.view_instance_id = getNextId();

  my.guest_instance_id = null;


  //
  // _public_
  //
  var webview_mutation_handler;        /* webview_mutation_handler(...); */
  var browser_plugin_mutation_handler; /* browser_plugin_mutation_handler(...); */

  var attached;                        /* attached([attach_value]) */
  var parse_attributes;                /* parse_attributes(); */
  var reset;                           /* reset(); */

  var api_loadUrl;                     /* api_loadUrl(url); */
  var api_go;                          /* api_go(index); */
  var api_reload;                      /* api_reload(ignore_cache); */
  
  //
  // _private_
  //
  var init;                            /* init(); */

  var is_plugin_in_render_tree;        /* is_plugin_in_render_tree(); */
  var build_attach_params;             /* build_attach_params(new_window); */
  var attach_window;                   /* attach_window(instance_id, new_window); */
  var create_guest;                    /* create_guest(); */
  var attr_src_parse;                  /* attr_src_parse(); */

  //
  // #### _that_
  //
  var that = {};

  /****************************************************************************/
  /* PRIVATE HELPERS */
  /****************************************************************************/
  // ### is_plugin_in_render_tree
  //
  // Returns whether <object> is in the render tree
  is_plugin_in_render_tree = function() {
    /* TODO(spolu): FixMe Chrome 39 */
    return (typeof my.browser_plugin_node[PLUGIN_METHOD_ATTACH] === 'function');
  };

  // ### build_attach_params
  //
  // Returns the attach params for plugin attachment
  build_attach_params = function(new_window) {
    var params = {
      'autosize': my.webview_node.hasAttribute(WEB_VIEW_ATTRIBUTE_AUTOSIZE),
      'instanceId': my.view_instance_id,
      'maxheight': my[WEB_VIEW_ATTRIBUTE_MAXHEIGHT],
      'maxwidth': my[WEB_VIEW_ATTRIBUTE_MAXWIDTH],
      'minheight': my[WEB_VIEW_ATTRIBUTE_MINHEIGHT],
      'minwidth': my[WEB_VIEW_ATTRIBUTE_MINWIDTH],
      // We don't need to navigate new window from here.
      'src': new_window ? undefined : my.src,
      // 'userAgentOverride': this.userAgentOverride
    };
    return params;
  }

  // ### attach_window
  //
  // Attaches the guest 
  // ```
  // @instance_id {string} the guest instance id
  // @new_window  {boolean}
  attach_window = function(instance_id, new_window) {
    my.guest_instance_id = instance_id;

    var params = build_attach_params(new_window);

    if(!is_plugin_in_render_tree()) {
      my.deferred_attach = { new_window: new_window };
      return false;
    }
    my.deferred_attach = null;
    return my.browser_plugin_node[PLUGIN_METHOD_ATTACH](instance_id, params);
  };

  // ### create_guest
  //
  // Triggers the creation of the guest
  create_guest = function() {
    var params = {};

    var instance_id = WebViewNatives.CreateGuest(params);
    if(!my.attached) {
      WebViewNatives.DestroyGuest(instance_id);
      return;
    }
    attach_window(instance_id, false);
  };

  // ### attr_src_parse
  //
  // Parses the `src` attribute and navigates if necessary
  attr_src_parse = function() {
    my.src = my.webview_node.getAttribute('src');

    if(!my.src) {
      return;
    }
    if(!my.guest_instance_id) {
      if(my.before_first_navigation) {
        my.before_first_navigation = false;
        /* First navigation. Create Guest. */
        create_guest();
      }
      return;
    }

    /* Navigate to my.src */
    api_loadUrl(my.src);
  };

  /****************************************************************************/
  /* WEBVIEW API */
  /****************************************************************************/
  // ### api_loadUrl
  //
  // Loads the specified URL (similar as updating `src`)
  // ```
  // @url {string} the url to load
  // ```
  api_loadUrl = function(url) {
    WebViewNatives.LoadUrl(my.guest_instance_id, url);
  };

  // ### api_go
  //
  // Navigates in history to the relative index
  // ```
  // @index {integer} the relative index
  // ```
  api_go = function(index) {
    WebViewNatives.Go(my.guest_instance_id, index);
  };

  // ### api_reload
  //
  // Reloads the webview content
  // ```
  // @ignore_cache {boolean} ignore cache
  // ```
  api_reload = function(ignore_cache) {
    WebViewNatives.Reload(my.guest_instance_id, ignore_cache ? true : false);
  };

  /****************************************************************************/
  /* PUBLIC METHODS */
  /****************************************************************************/
  // ### webview_mutation_handler
  //
  // Handles mutations to the webview and updates the browser_plugin properties
  // accordingly. In turn, updating a BrowserPlugin property will update the 
  // corresponding BrowserPlugin attribute, if necessary. 
  // See BrowserPlugin::UpdateDOMAttribute for more details.
  // ```
  // @name      {string} the attribute name
  // @old_value {value} the old value
  // @new_value {value} the new value
  // ```
  webview_mutation_handler = function(name, old_value, new_value) {
    /* TODO(spolu): see handleWebviewAttributeMutation */
    console.log("HANDLER: " + name + " " + old_value + " " + new_value);
  };

  // ### browser_plugin_mutation_handler
  //
  // ```
  // @name      {string} the attribute name
  // @old_value {value} the old value
  // @new_value {value} the new value
  // ```
  browser_plugin_mutation_handler = function(name, old_value, new_value) {
    /* TODO(spolu): see handleBrowserPluginAttributeMutation */

    /* TODO(spolu): FixMe Chrome 39 */
    if (name == 'internalbindings' && !old_value && new_value) {
      my.browser_plugin_node.removeAttribute('internalbindings');

      /* If we already created the guest but the plugin was not in the render */
      /* tree, then we attach the plugin now.                                 */
      if(my.guest_instance_id) {
        var new_window =
          my.deferred_attach ? my.deferred_attach.new_window : false;
        my.deferred_attach = null;
        var params = build_attach_params(new_window);
        my.browser_plugin_node[PLUGIN_METHOD_ATTACH](instance_id, params);
      }
    }
  };

  // ### attached
  //
  // Getter/Setter for the attached field
  // ```
  // @attached {boolean}
  // ```
  attached = function(attached_value) {
    if(typeof attached_value !== 'boolean') {
      return my.attached;
    }
    my.attached = attached_value;
    return my.attached;
  }

  // ### parse_attributes
  //
  // Parses the attributes if the element is attached
  parse_attributes = function() {
    if(!my.attached) {
      return;
    }
    attr_src_parse();
  };

  // ### reset
  //
  // Resets the state upon detachment of the element
  reset = function() {
    if(my.guest_instance_id) {
      WebViewNatives.DestroyGuest(my.guest_instance_id);
      my.guest_instance_id = null;
      my.before_first_navigation = true;
    }
  };
  
  /****************************************************************************/
  /* INITIALIZATION */
  /****************************************************************************/
  // ### init
  //
  // Initializes the webview object and registers itself as internal hidden
  // value for the node
  init = function() {
    privates(my.webview_node).internal = that;

    // We create BrowserPlugin as a custom element in order to observe changes
    // to attributes synchronously.
    my.browser_plugin_node = new window.BrowserPlugin();
    privates(my.browser_plugin_node).internal = that;

    /* We create the shadow root for this element and append the browser */
    /* plugin node to it.                                                */
    my.webview_node.createShadowRoot().appendChild(my.browser_plugin_node);

    /* Set up webview autoresize attributes */
    AUTO_SIZE_ATTRIBUTES.forEach(function(attr) {
      my[attr] = my.webview_node.getAttribute(attr);
      Object.defineProperty(my.webview_node, attr, {
        get: function() {
          return my[attr];
        },
        set: function(value) {
          my.webview_node.setAttribute(attr, value);
        },
        enumerable: true
      });
    });

    /* Set up webview attributes */
    WEB_VIEW_ATTRIBUTES.forEach(function(attr) {
      if(my.webview_node.hasAttribute(attr)) {
        my.browser_plugin_node.setAttribute(
          attr, my.webview_node.getAttribute(attr));
      } 
      else if(my.webview_node[attr]){
        /* Reading property using has/getAttribute does not work on */
        /* document.DOMContentLoaded event (but works on            */
        /* window.DOMContentLoaded event).                          */
        /* So copy from property if copying from attribute fails.   */
        my.browser_plugin_node.setAttribute(
          attr, my.webview_node[attr]);
      }
      Object.defineProperty(my.webview_node, attr, {
        get: function() {
          if(my.browser_plugin_node.hasOwnProperty(attr)) {
            return my.browser_plugin_node[attr];
          } 
          else {
            return my.browser_plugin_node.getAttribute(attr);
          }
        },
        set: function(value) {
          if (my.browser_plugin_node.hasOwnProperty(attr)) {
            /* Give the BrowserPlugin first stab at the attribute so that it */
            /* can throw an exception if there is a problem. This attribute  */
            /* will then be propagated back to the <webview>.                */
            my.browser_plugin_node[attr] = value;
          } 
          else {
            my.browser_plugin_node.setAttribute(attr, value);
          }
        },
        enumerable: true
      });
    }, this);

    /* <webview> src does not quite behave the same as BrowserPlugin src, and */
    /* so we don't simply keep the two in sync.                               */
    my.src = my.webview_node.getAttribute('src');
    Object.defineProperty(my.webview_node, 'src', {
      get: function() {
        return my.src;
      },
      set: function(value) {
        my.webview_node.setAttribute('src', value);
      },
      enumerable: true
    });

    Object.defineProperty(my.webview_node, 'name', {
      get: function() {
        return self.name;
      },
      set: function(value) {
        my.webview_node.setAttribute('name', value);
      },
      enumerable: true
    });

    /* We cannot use {writable: true} property descriptor because we want a */
    /* dynamic getter value.                                                */
    Object.defineProperty(my.webview_node, 'contentWindow', {
      get: function() {
        if(my.browser_plugin_node.contentWindow) {
          return my.browser_plugin_node.contentWindow;
        }
        window.console.error(ERROR_MSG_CONTENTWINDOW_NOT_AVAILABLE);
      },
      // No setter.
      enumerable: true
    });

    /* The purpose of this mutation observer is to catch assignment to the src */
    /* attribute without any changes to its value. This is useful in the case  */
    /* where the webview guest has crashed and navigating to the same address  */
    /* spawns off a new process.                                               */
    my.src_observer = new MutationObserver(function(mutations) {
      mutations.forEach(function(mutation) {
        var old_value = mutation.oldValue;
        var new_value = my.webview_node.getAttribute(mutation.attributeName);
        if(old_value != new_value) {
          return;
        }
        webview_mutation_handler(mutation.attributeName, old_value, new_value);
      });
    });
    my.src_observer.observe(my.webview_node, {
      attributes: true,
      attributeOldValue: true,
      attributeFilter: ['src']
    });

    /* <webview> needs a tabIndex in order to be focusable. */
    if(!my.webview_node.hasAttribute('tabIndex')) {
      my.webview_node.setAttribute('tabIndex', -1);
    }
    /* Setup the focus propagation from <webview> to the BrowserPlugin. */
    my.webview_node.addEventListener('focus', function(e) {
      my.browser_plugin_node.focus();
    });
    my.webview_node.addEventListener('blur', function(e) {
      my.browser_plugin_node.blur();
    });

    /* TODO(spolu): Register Events */
    
    /* Finally triggers the guest creation and first navigation. If the */
    /* element is attached.                                             */
    parse_attributes();
  };
  
  that.webview_mutation_handler = webview_mutation_handler;
  that.browser_plugin_mutation_handler = browser_plugin_mutation_handler;

  that.attached = attached;
  that.parse_attributes = parse_attributes;
  that.reset = reset;

  that.api_loadUrl = api_loadUrl;
  that.api_go = api_go;
  that.api_reload = api_reload;

  init();

  return that;
};


/******************************************************************************/
/* ELEMENT REGISTRATION */
/******************************************************************************/
// ### registerBrowserPluginElement
//
// Registers browser plugin <object> custom element.
function registerBrowserPluginElement() {
  var proto = Object.create(HTMLObjectElement.prototype);

  proto.createdCallback = function() {
    this.setAttribute('type', 'application/browser-plugin');
    this.setAttribute('id', 'browser-plugin-' + getNextId());
    // The <object> node fills in the <webview> container.
    this.style.width = '100%';
    this.style.height = '100%';
  };

  proto.attributeChangedCallback = function(name, old_value, new_value) {
    var internal = privates(this).internal;
    if(!internal) {
      return;
    }
    internal.browser_plugin_mutation_handler(name, old_value, new_value);
  };

  proto.attachedCallback = function() {
    // Load the plugin immediately.
    var unused = this.nonExistentAttribute;
  };

  window.BrowserPlugin =
      DocumentNatives.RegisterElement('browserplugin', { extends: 'object',
                                                         prototype: proto });

  // Delete the callbacks so developers cannot call them and produce unexpected
  // behavior.
  delete proto.createdCallback;
  delete proto.attachedCallback;
  delete proto.detachedCallback;
  delete proto.attributeChangedCallback;
}



// ### registerWebViewElement
//
// Registers <webview> custom element and sets up method forwarding to the 
// hidden internal value associated with it
function registerWebViewElement() {
  var proto = Object.create(HTMLElement.prototype);

  proto.createdCallback = function() {
    webview({ node: this });
  };

  proto.attributeChangedCallback = function(name, old_value, new_value) {
    var internal = privates(this).internal;
    if(!internal) {
      return;
    }
    internal.webview_mutation_handler(name, old_value, new_value);
  };

  proto.detachedCallback = function() {
    var internal = privates(this).internal;
    if(!internal) {
      return;
    }
    internal.attached(false);
    internal.reset();
  };

  proto.attachedCallback = function() {
    var internal = privates(this).internal;
    if(!internal) {
      return;
    }
    if(!internal.attached()) {
      internal.attached(true)
      internal.parse_attributes();
    }
  };

  var methods = [
    /*
    'back',
    'forward',
    'canGoBack',
    'canGoForward',
    */
    'loadUrl',
    'go',
    'reload',
    /*
    'stop',
    'clearData',
    'getProcessId',
    'getZoom',
    'setZoom',
    'print',
    'find',
    'stopFinding',
    'terminate',
    'executeScript',
    'insertCSS',
    'getUserAgent',
    'isUserAgentOverridden',
    'setUserAgentOverride'
    */
  ];

  // Forward proto.foo* method calls to WebViewInternal.foo*.
  for(var i = 0; methods[i]; ++i) {
    var createHandler = function(m) {
      return function(var_args) {
        var internal = privates(this).internal;
        return $Function.apply(internal['api_' + m], internal, arguments);
      };
    };
    proto[methods[i]] = createHandler(methods[i]);
  }

  window.WebView =
      DocumentNatives.RegisterElement('webview', { prototype: proto });

  // Delete the callbacks so developers cannot call them and produce unexpected
  // behavior.
  delete proto.createdCallback;
  delete proto.attachedCallback;
  delete proto.detachedCallback;
  delete proto.attributeChangedCallback;
}


window.addEventListener('readystatechange', function listener(event) {
  if(document.readyState == 'loading')
    return;
  registerBrowserPluginElement();
  registerWebViewElement();
  window.removeEventListener(event.type, listener, true);
}, true);

