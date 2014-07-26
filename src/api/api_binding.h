// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_BROWSER_API_API_BINDING_H_
#define EXO_BROWSER_API_API_BINDING_H_

namespace exo_browser {

// ## ApiBinding
//
// Exposes the interface of an object binded to the API
class ApiBinding {
public:
  typedef base::Callback<void(std::string& error, Value* result)> 
    MethodCallback;

  /****************************************************************************/
  /* VIRTUAL INTERFACE */
  /****************************************************************************/
  virtual void LocalCall(std::string& method, 
                         Value* args, const &MethodCallback callback) = 0;

protected:
  /****************************************************************************/
  /* PROTECTED INTERFACE */
  /****************************************************************************/
  void RemoteCall(std::string& method, 
                  Value* args, const &MethodCallback callback);
  void Emit(std::string& type, Value* event);

  ApiBinding(const std::string& type);

private
  virtual ~ApiBinding();

  std::string      type_;
  std::string      id_;

  DISALLOW_COPY_AND_ASSIGN(ApiBinding);
};

// ## ApiBindingFactory
//
// Factory object used to generate typed binding objects
class ApiBindingFactory {
public:
  virtual ~ApiBindingFactory() {}

  virtual ApiBinding* Create() = 0;
};

} // namespace exo_browser
  
#endif // EXO_BROWSER_API_API_BINDING_H_
