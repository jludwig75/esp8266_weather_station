#pragma once

#include <ESP8266WebServer.h>


template<typename _T>
class MethodRequestHandler : public RequestHandler {
public:
	typedef void (_T::*HandlerMethod)();
	MethodRequestHandler(_T *handler_class, HandlerMethod fn, HandlerMethod ufn, const char* uri, HTTPMethod method)
		: _handler_class(handler_class)
		, _fn(fn)
		, _ufn(ufn)
		, _uri(uri)
		, _method(method)
	{
	}

	bool canHandle(HTTPMethod requestMethod, String requestUri) override {
		if (_method != HTTP_ANY && _method != requestMethod)
			return false;

		if (requestUri != _uri)
			return false;

		return true;
	}

	bool canUpload(String requestUri) override {
		if (!_ufn || !canHandle(HTTP_POST, requestUri))
			return false;

		return true;
	}

	bool handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) override {
		if (!canHandle(requestMethod, requestUri))
			return false;

		(*_handler_class.*_fn)();
		return true;
	}

	void upload(ESP8266WebServer& server, String requestUri, HTTPUpload& upload) override {
		if (canUpload(requestUri))
			(*_handler_class.*_ufn)();
	}

protected:
	_T *_handler_class;
	HandlerMethod _fn;
	HandlerMethod _ufn;
	String _uri;
	HTTPMethod _method;
};


template<typename _T>
class OOWebServer : public ESP8266WebServer
{
public:
	OOWebServer(IPAddress addr, int port) : ESP8266WebServer(addr, port)
	{
	}

	OOWebServer(int port) : ESP8266WebServer(port)
	{
	}

protected:
	void on(const char* uri, void (_T::*fn)())
	{
		on(uri, HTTP_ANY, fn);
	}
	void on(const char* uri, HTTPMethod method, void (_T::*fn)())
	{
		on(uri, method, fn, NULL);
	}
	void on(const char* uri, HTTPMethod method, void (_T::*fn)(), void (_T::*ufn)())
	{
		addHandler(new MethodRequestHandler<_T>((_T *)this, fn, ufn, uri, method));
	}
};

