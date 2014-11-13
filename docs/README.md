
A typical Thrust app is composed of two main components. The platform code
which is language specific and calls into one of Thrust's language binding,
and the HTML5 client code which is executed by Thrusts windows.

The platform code is generally in charge of serving the client code locally and
provide an API for it to interact with. As the platform code is language 
specific, the API reference only describe objects and available methods, using
a pseudocode format, as exposed by Thrust standard I/O API for language bindings 
to interact with. Please refer to your specific language bindings for a more 
specific documentation and syntax.

### API Reference

Platform code objects:

- [window](api/window.md)
- [session](api/session.md)
- [menu](api/menu.md)

Client code modules:

- remote (coming soon)

Client custom DOM elements:

- [`<webview>` tag](api/webview.md)

### Language Bindings Documenatation

- node-thrust
- [go-thrust](https://github.com/miketheprogrammer/go-thrust/tree/master/doc)
- pythrust
