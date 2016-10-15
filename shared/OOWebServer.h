#pragma once

#include <ESP8266WebServer.h>
#include <FS.h>


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

	String uri() const
	{
		return _uri;
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
	OOWebServer(const String &templates_base_dir, IPAddress addr, int port) : _templates_base_dir(templates_base_dir), _page_info_list(NULL), ESP8266WebServer(addr, port)
	{
	}

	OOWebServer(const String &templates_base_dir, int port) : _templates_base_dir(templates_base_dir), _page_info_list(NULL),   ESP8266WebServer(port)
	{
	}

    virtual void server_begin() = 0;
    
    void begin()
    {
        ESP8266WebServer::begin();
        server_begin();
        loadPageTemplates();
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
        add_page_info(uri); // TODO: Move to on()
    }

    String get_current_page_template() const
    {
        return get_page_template(_currentUri);
    }

    String get_page_template(const String & uri) const
    {
        for (html_page_info *page_info = _page_info_list; page_info != NULL; page_info = page_info->_next)
        {
            if (page_info->_uri == uri)
            {
                return page_info->_template_data;
            }
        }

        return "";
    }

private:
    struct html_page_info
    {
        html_page_info(String uri) :
            _next(NULL),
            _uri(uri)
        {

        }
        html_page_info *_next;
        String _uri;
        String _template_data;
    };

    void loadPageTemplates()
    {
        Serial.println("Loading templates...");
        // Load HTML templates
        for (html_page_info *page_info = _page_info_list; page_info != NULL; page_info = page_info->_next)
        {
            String uri = page_info->_uri;

            String templateName = "";
            if (uri == "/")
            {
                templateName = "/index";
            }
            else
            {
                templateName = uri;
            }

            templateName += ".html";

            LoadTemplate(uri, templateName);
        }
    }

    void LoadTemplate(const String & uri, const String & templateName)
    {
        html_page_info *page_info = get_page_info(uri);
        if (!page_info)
        {
            return;
        }

        String template_fil_name = _templates_base_dir + templateName;
        File template_file = SPIFFS.open(template_fil_name, "r");
        if (template_file)
        {
            Serial.print("Successfully opened file '");
            Serial.print(template_fil_name.c_str());
            Serial.println("'\n");
            size_t template_size = template_file.size();
            char *template_contents = new char[template_size + 1];
            if (template_contents)
            {
                template_file.readBytes(template_contents, template_size);
                template_contents[template_size] = 0;

                page_info->_template_data = String(template_contents);
            }

            template_file.close();
        }
        else
        {
            Serial.print("Failed to open file '");
            Serial.print(template_fil_name.c_str());
            Serial.println("'\n");
        }
    }

    void add_page_info(const String & uri)
    {
        if (get_page_info(uri))
        {
            // Already added page_info.
            return;
        }

        html_page_info *page_info = new html_page_info(uri);
        if (page_info)
        {
            if (_page_info_list)
            {
                page_info->_next = _page_info_list;
            }

            _page_info_list = page_info;
        }
    }

    html_page_info *get_page_info(const String & uri)
    {
        for (html_page_info *page_info = _page_info_list; page_info != NULL; page_info = page_info->_next)
        {
            if (page_info->_uri == uri)
            {
                return page_info;
            }
        }

        return NULL;
    }

    void load_page_templates(const String & uri, const String & page_template)
    {
        for (html_page_info *page_info = _page_info_list; page_info != NULL; page_info = page_info->_next)
        {
            if (page_info->_uri == uri)
            {
                page_info->_template_data = page_template;
            }
        }
    }

    String _templates_base_dir;
    html_page_info *_page_info_list;
};

