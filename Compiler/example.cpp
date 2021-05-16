#include <memory>
#include <cstdlib>
#include <restbed>
#include <fstream>
#include <string.h>
#include "compiler.cpp"

using namespace std;
using namespace restbed;

string evaluate(unsigned long int ln, const unsigned char *uc_param)
{
    if(ln == 0) 
        return "";
    time_t now = time(0);
    struct tm* tm = localtime(&now);    
    ofstream out( "logfile.txt",ios::app);
    out << tm->tm_year << '/' << tm->tm_mon << '/' << tm->tm_mday
         << ' ' << tm->tm_hour << ':' << tm->tm_min << ':' << tm->tm_sec << ": ";
    string string_param(reinterpret_cast< char const* >(uc_param));
    out << ln << "| "<< string_param.substr(0, ln) << endl;
    out.close();
    
    string response_string = analyze(string_param.substr(0, ln));

    return response_string;
}

void post_method_handler(const shared_ptr<Session> session)
{
    const auto request = session->get_request( );
    string content_length_str = request->get_header("Content-Length");
    int content_length = stoi(content_length_str);
    fprintf(stdout, "Length: %d, ", content_length);
    session->fetch(content_length, [](const shared_ptr<Session> session, const Bytes& body)
    {
        //fprintf(stdout, "Message: %.*s\n", static_cast<int>(body.size()), body.data());
        string r = evaluate(body.size(), body.data());
        session->close(OK, r, {{"Content-Length", to_string(r.length())}});
    } );
}

int main(const int, const char**)
{
    auto resource = make_shared<Resource>();
    resource->set_path("/cmml");
    resource->set_method_handler("POST", post_method_handler);

    auto settings = make_shared<Settings>();
    settings->set_bind_address("127.0.0.1");
    settings->set_port(3001);
    //settings->set_default_header("Access-Control-Allow-Headers", "Authorization");
    settings->set_default_header("Access-Control-Allow-Origin", "*");
    // settings->set_default_header("Access-Control-Request-Method", "POST");
    // settings->set_default_header("Access-Control-Request-Headers", "X-Requested-With");
    // settings->set_default_header("Access-Control-Allow-Credentials", "true");
    // settings->set_default_header("Access-Control-Max-Age", "3600");
    // settings->set_default_header("Access-Control-Expose-Headers", "X-Pagination-Current-Page");
    settings->set_default_header("Connection", "close");
    
    Service service;
    service.publish(resource);
    service.start(settings);

    return EXIT_SUCCESS;
}
