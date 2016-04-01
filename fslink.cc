#include <node.h>
#include <nan.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>

#define BUFFER_SIZE 4086

namespace fslink {

    using node::AtExit;
    using v8::Exception;
    using v8::FunctionCallbackInfo;
    using v8::Isolate;
    using v8::Local;
    using v8::Number;
    using v8::Object;
    using v8::String;
    using v8::Value;

    void ThrowError(Isolate* isolate, const char *message){
        isolate->ThrowException(Exception::Error(
            String::NewFromUtf8(isolate, message)));
        return;
    };

    void ThrowTypeError(Isolate* isolate, const char *message){
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, message)));
        return;
    };

    int sok;
    int ret;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char *nbuffer = (char *)malloc(BUFFER_SIZE);
    const char* host;
    int port;


    void Connect(const FunctionCallbackInfo<Value>& args){
        Isolate* isolate = args.GetIsolate();
        if(args.Length() < 2)
            return ThrowError(isolate, "Connect() requires 2 arguments");

        if(!args[0]->IsString())
            return ThrowTypeError( isolate, "Connect() argument 1 must be an IP or domain name");

        if(!args[1]->IsNumber())
            return ThrowTypeError( isolate, "Connect() argument 2 must be a port number");

        v8::String::Utf8Value param1(args[0]->ToString());
        std::string shost = std::string(*param1);
        host = shost.c_str();

        port = args[1]->NumberValue();

        sok = socket(AF_INET, SOCK_STREAM, 0);
        if(sok < 0 ) ThrowError(isolate, "Connect# socket openning error");
        server = gethostbyname(host);
        if(server == NULL) ThrowError(isolate, "Connect# gethostbyname error");
        bzero((char *) &server_addr, sizeof(server_addr));

        server_addr.sin_family = AF_INET;
        bcopy(
            (char *) server->h_addr,
            (char *)&server_addr.sin_addr.s_addr,
            server->h_length
        );
        server_addr.sin_port = htons(port);
        ret = connect(sok, (struct sockaddr *) &server_addr, sizeof(server_addr));
        if(ret < 0) ThrowError(isolate, strerror(errno));

        args.GetReturnValue().Set(sok);
    };


    void Write(const FunctionCallbackInfo<Value>& args){
        Isolate* isolate = args.GetIsolate();
        if(args.Length() < 1)
            return ThrowError(isolate, "Write# Write() requires 1 argument");
        if(!args[0]->IsString())
            return ThrowTypeError( isolate, "Write# Write() requires a string/buffer argument");

        v8::String::Utf8Value vBuffer(args[0]->ToString());
        std::string bString = std::string(*vBuffer);
        const char* buffer = bString.c_str();

        ret = write(sok, buffer, bString.length() );
        if(ret < 0)
            ThrowError(isolate, strerror(errno));

        args.GetReturnValue().Set(ret);
    };

    void Read(const FunctionCallbackInfo<Value>& args){
        Isolate* isolate = args.GetIsolate();
        bzero(nbuffer, BUFFER_SIZE);
        ret = read(sok, nbuffer, BUFFER_SIZE );
        if(ret < 0)
            ThrowError(isolate, strerror(errno));

        Local<String> retval = String::NewFromUtf8(isolate, nbuffer );
        args.GetReturnValue().Set( retval );
    };

    void Disconnect(const FunctionCallbackInfo<Value>& args){
        if(nbuffer) free(nbuffer);
        if(sok) close(sok);
    };


    static void at_exit_callback(void *arg){
        printf("AtExit executing ....%s\n", "");
        if(nbuffer) free(nbuffer);
        if(sok) close(sok);
    };

    void Init(Local<Object> exports) {

        NODE_SET_METHOD(exports, "Connect", Connect);
        NODE_SET_METHOD(exports, "Write", Write);
        NODE_SET_METHOD(exports, "Read", Read);
        NODE_SET_METHOD(exports, "Disconnect", Disconnect);

        AtExit(at_exit_callback, exports->GetIsolate());
    };


    NODE_MODULE(fslink, Init)
}
