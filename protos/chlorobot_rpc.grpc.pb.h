// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: chlorobot_rpc.proto
#ifndef GRPC_chlorobot_5frpc_2eproto__INCLUDED
#define GRPC_chlorobot_5frpc_2eproto__INCLUDED

#include "chlorobot_rpc.pb.h"

#include <functional>
#include <grpcpp/generic/async_generic_service.h>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/support/client_callback.h>
#include <grpcpp/client_context.h>
#include <grpcpp/completion_queue.h>
#include <grpcpp/support/message_allocator.h>
#include <grpcpp/support/method_handler.h>
#include <grpcpp/impl/codegen/proto_utils.h>
#include <grpcpp/impl/rpc_method.h>
#include <grpcpp/support/server_callback.h>
#include <grpcpp/impl/codegen/server_callback_handlers.h>
#include <grpcpp/server_context.h>
#include <grpcpp/impl/service_type.h>
#include <grpcpp/impl/codegen/status.h>
#include <grpcpp/support/stub_options.h>
#include <grpcpp/support/sync_stream.h>

// A Chlorobot IRC RPC interface
class ChlorobotRPC final {
 public:
  static constexpr char const* service_full_name() {
    return "ChlorobotRPC";
  }
  class StubInterface {
   public:
    virtual ~StubInterface() {}
    // Listens in on the IRC interface
    //
    // Streams parsed IRC packets
    std::unique_ptr< ::grpc::ClientReaderInterface< ::ChlorobotPacket>> Listen(::grpc::ClientContext* context, const ::ChlorobotAuthentication& request) {
      return std::unique_ptr< ::grpc::ClientReaderInterface< ::ChlorobotPacket>>(ListenRaw(context, request));
    }
    std::unique_ptr< ::grpc::ClientAsyncReaderInterface< ::ChlorobotPacket>> AsyncListen(::grpc::ClientContext* context, const ::ChlorobotAuthentication& request, ::grpc::CompletionQueue* cq, void* tag) {
      return std::unique_ptr< ::grpc::ClientAsyncReaderInterface< ::ChlorobotPacket>>(AsyncListenRaw(context, request, cq, tag));
    }
    std::unique_ptr< ::grpc::ClientAsyncReaderInterface< ::ChlorobotPacket>> PrepareAsyncListen(::grpc::ClientContext* context, const ::ChlorobotAuthentication& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncReaderInterface< ::ChlorobotPacket>>(PrepareAsyncListenRaw(context, request, cq));
    }
    // Sends a packet to the IRC interface
    //
    // Must send authentication data for security
    // Returns an acknowledgement token indicating if it was successful
    virtual ::grpc::Status Send(::grpc::ClientContext* context, const ::ChlorobotRequest& request, ::ChlorobotAcknowledgement* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::ChlorobotAcknowledgement>> AsyncSend(::grpc::ClientContext* context, const ::ChlorobotRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::ChlorobotAcknowledgement>>(AsyncSendRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::ChlorobotAcknowledgement>> PrepareAsyncSend(::grpc::ClientContext* context, const ::ChlorobotRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::ChlorobotAcknowledgement>>(PrepareAsyncSendRaw(context, request, cq));
    }
    class async_interface {
     public:
      virtual ~async_interface() {}
      // Listens in on the IRC interface
      //
      // Streams parsed IRC packets
      virtual void Listen(::grpc::ClientContext* context, const ::ChlorobotAuthentication* request, ::grpc::ClientReadReactor< ::ChlorobotPacket>* reactor) = 0;
      // Sends a packet to the IRC interface
      //
      // Must send authentication data for security
      // Returns an acknowledgement token indicating if it was successful
      virtual void Send(::grpc::ClientContext* context, const ::ChlorobotRequest* request, ::ChlorobotAcknowledgement* response, std::function<void(::grpc::Status)>) = 0;
      virtual void Send(::grpc::ClientContext* context, const ::ChlorobotRequest* request, ::ChlorobotAcknowledgement* response, ::grpc::ClientUnaryReactor* reactor) = 0;
    };
    typedef class async_interface experimental_async_interface;
    virtual class async_interface* async() { return nullptr; }
    class async_interface* experimental_async() { return async(); }
   private:
    virtual ::grpc::ClientReaderInterface< ::ChlorobotPacket>* ListenRaw(::grpc::ClientContext* context, const ::ChlorobotAuthentication& request) = 0;
    virtual ::grpc::ClientAsyncReaderInterface< ::ChlorobotPacket>* AsyncListenRaw(::grpc::ClientContext* context, const ::ChlorobotAuthentication& request, ::grpc::CompletionQueue* cq, void* tag) = 0;
    virtual ::grpc::ClientAsyncReaderInterface< ::ChlorobotPacket>* PrepareAsyncListenRaw(::grpc::ClientContext* context, const ::ChlorobotAuthentication& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::ChlorobotAcknowledgement>* AsyncSendRaw(::grpc::ClientContext* context, const ::ChlorobotRequest& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::ChlorobotAcknowledgement>* PrepareAsyncSendRaw(::grpc::ClientContext* context, const ::ChlorobotRequest& request, ::grpc::CompletionQueue* cq) = 0;
  };
  class Stub final : public StubInterface {
   public:
    Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());
    std::unique_ptr< ::grpc::ClientReader< ::ChlorobotPacket>> Listen(::grpc::ClientContext* context, const ::ChlorobotAuthentication& request) {
      return std::unique_ptr< ::grpc::ClientReader< ::ChlorobotPacket>>(ListenRaw(context, request));
    }
    std::unique_ptr< ::grpc::ClientAsyncReader< ::ChlorobotPacket>> AsyncListen(::grpc::ClientContext* context, const ::ChlorobotAuthentication& request, ::grpc::CompletionQueue* cq, void* tag) {
      return std::unique_ptr< ::grpc::ClientAsyncReader< ::ChlorobotPacket>>(AsyncListenRaw(context, request, cq, tag));
    }
    std::unique_ptr< ::grpc::ClientAsyncReader< ::ChlorobotPacket>> PrepareAsyncListen(::grpc::ClientContext* context, const ::ChlorobotAuthentication& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncReader< ::ChlorobotPacket>>(PrepareAsyncListenRaw(context, request, cq));
    }
    ::grpc::Status Send(::grpc::ClientContext* context, const ::ChlorobotRequest& request, ::ChlorobotAcknowledgement* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::ChlorobotAcknowledgement>> AsyncSend(::grpc::ClientContext* context, const ::ChlorobotRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::ChlorobotAcknowledgement>>(AsyncSendRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::ChlorobotAcknowledgement>> PrepareAsyncSend(::grpc::ClientContext* context, const ::ChlorobotRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::ChlorobotAcknowledgement>>(PrepareAsyncSendRaw(context, request, cq));
    }
    class async final :
      public StubInterface::async_interface {
     public:
      void Listen(::grpc::ClientContext* context, const ::ChlorobotAuthentication* request, ::grpc::ClientReadReactor< ::ChlorobotPacket>* reactor) override;
      void Send(::grpc::ClientContext* context, const ::ChlorobotRequest* request, ::ChlorobotAcknowledgement* response, std::function<void(::grpc::Status)>) override;
      void Send(::grpc::ClientContext* context, const ::ChlorobotRequest* request, ::ChlorobotAcknowledgement* response, ::grpc::ClientUnaryReactor* reactor) override;
     private:
      friend class Stub;
      explicit async(Stub* stub): stub_(stub) { }
      Stub* stub() { return stub_; }
      Stub* stub_;
    };
    class async* async() override { return &async_stub_; }

   private:
    std::shared_ptr< ::grpc::ChannelInterface> channel_;
    class async async_stub_{this};
    ::grpc::ClientReader< ::ChlorobotPacket>* ListenRaw(::grpc::ClientContext* context, const ::ChlorobotAuthentication& request) override;
    ::grpc::ClientAsyncReader< ::ChlorobotPacket>* AsyncListenRaw(::grpc::ClientContext* context, const ::ChlorobotAuthentication& request, ::grpc::CompletionQueue* cq, void* tag) override;
    ::grpc::ClientAsyncReader< ::ChlorobotPacket>* PrepareAsyncListenRaw(::grpc::ClientContext* context, const ::ChlorobotAuthentication& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::ChlorobotAcknowledgement>* AsyncSendRaw(::grpc::ClientContext* context, const ::ChlorobotRequest& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::ChlorobotAcknowledgement>* PrepareAsyncSendRaw(::grpc::ClientContext* context, const ::ChlorobotRequest& request, ::grpc::CompletionQueue* cq) override;
    const ::grpc::internal::RpcMethod rpcmethod_Listen_;
    const ::grpc::internal::RpcMethod rpcmethod_Send_;
  };
  static std::unique_ptr<Stub> NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());

  class Service : public ::grpc::Service {
   public:
    Service();
    virtual ~Service();
    // Listens in on the IRC interface
    //
    // Streams parsed IRC packets
    virtual ::grpc::Status Listen(::grpc::ServerContext* context, const ::ChlorobotAuthentication* request, ::grpc::ServerWriter< ::ChlorobotPacket>* writer);
    // Sends a packet to the IRC interface
    //
    // Must send authentication data for security
    // Returns an acknowledgement token indicating if it was successful
    virtual ::grpc::Status Send(::grpc::ServerContext* context, const ::ChlorobotRequest* request, ::ChlorobotAcknowledgement* response);
  };
  template <class BaseClass>
  class WithAsyncMethod_Listen : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_Listen() {
      ::grpc::Service::MarkMethodAsync(0);
    }
    ~WithAsyncMethod_Listen() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Listen(::grpc::ServerContext* /*context*/, const ::ChlorobotAuthentication* /*request*/, ::grpc::ServerWriter< ::ChlorobotPacket>* /*writer*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestListen(::grpc::ServerContext* context, ::ChlorobotAuthentication* request, ::grpc::ServerAsyncWriter< ::ChlorobotPacket>* writer, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncServerStreaming(0, context, request, writer, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_Send : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_Send() {
      ::grpc::Service::MarkMethodAsync(1);
    }
    ~WithAsyncMethod_Send() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Send(::grpc::ServerContext* /*context*/, const ::ChlorobotRequest* /*request*/, ::ChlorobotAcknowledgement* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestSend(::grpc::ServerContext* context, ::ChlorobotRequest* request, ::grpc::ServerAsyncResponseWriter< ::ChlorobotAcknowledgement>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(1, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  typedef WithAsyncMethod_Listen<WithAsyncMethod_Send<Service > > AsyncService;
  template <class BaseClass>
  class WithCallbackMethod_Listen : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithCallbackMethod_Listen() {
      ::grpc::Service::MarkMethodCallback(0,
          new ::grpc::internal::CallbackServerStreamingHandler< ::ChlorobotAuthentication, ::ChlorobotPacket>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::ChlorobotAuthentication* request) { return this->Listen(context, request); }));
    }
    ~WithCallbackMethod_Listen() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Listen(::grpc::ServerContext* /*context*/, const ::ChlorobotAuthentication* /*request*/, ::grpc::ServerWriter< ::ChlorobotPacket>* /*writer*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerWriteReactor< ::ChlorobotPacket>* Listen(
      ::grpc::CallbackServerContext* /*context*/, const ::ChlorobotAuthentication* /*request*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithCallbackMethod_Send : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithCallbackMethod_Send() {
      ::grpc::Service::MarkMethodCallback(1,
          new ::grpc::internal::CallbackUnaryHandler< ::ChlorobotRequest, ::ChlorobotAcknowledgement>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::ChlorobotRequest* request, ::ChlorobotAcknowledgement* response) { return this->Send(context, request, response); }));}
    void SetMessageAllocatorFor_Send(
        ::grpc::MessageAllocator< ::ChlorobotRequest, ::ChlorobotAcknowledgement>* allocator) {
      ::grpc::internal::MethodHandler* const handler = ::grpc::Service::GetHandler(1);
      static_cast<::grpc::internal::CallbackUnaryHandler< ::ChlorobotRequest, ::ChlorobotAcknowledgement>*>(handler)
              ->SetMessageAllocator(allocator);
    }
    ~WithCallbackMethod_Send() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Send(::grpc::ServerContext* /*context*/, const ::ChlorobotRequest* /*request*/, ::ChlorobotAcknowledgement* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* Send(
      ::grpc::CallbackServerContext* /*context*/, const ::ChlorobotRequest* /*request*/, ::ChlorobotAcknowledgement* /*response*/)  { return nullptr; }
  };
  typedef WithCallbackMethod_Listen<WithCallbackMethod_Send<Service > > CallbackService;
  typedef CallbackService ExperimentalCallbackService;
  template <class BaseClass>
  class WithGenericMethod_Listen : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_Listen() {
      ::grpc::Service::MarkMethodGeneric(0);
    }
    ~WithGenericMethod_Listen() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Listen(::grpc::ServerContext* /*context*/, const ::ChlorobotAuthentication* /*request*/, ::grpc::ServerWriter< ::ChlorobotPacket>* /*writer*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_Send : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_Send() {
      ::grpc::Service::MarkMethodGeneric(1);
    }
    ~WithGenericMethod_Send() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Send(::grpc::ServerContext* /*context*/, const ::ChlorobotRequest* /*request*/, ::ChlorobotAcknowledgement* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithRawMethod_Listen : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_Listen() {
      ::grpc::Service::MarkMethodRaw(0);
    }
    ~WithRawMethod_Listen() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Listen(::grpc::ServerContext* /*context*/, const ::ChlorobotAuthentication* /*request*/, ::grpc::ServerWriter< ::ChlorobotPacket>* /*writer*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestListen(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncWriter< ::grpc::ByteBuffer>* writer, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncServerStreaming(0, context, request, writer, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawMethod_Send : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_Send() {
      ::grpc::Service::MarkMethodRaw(1);
    }
    ~WithRawMethod_Send() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Send(::grpc::ServerContext* /*context*/, const ::ChlorobotRequest* /*request*/, ::ChlorobotAcknowledgement* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestSend(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(1, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawCallbackMethod_Listen : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawCallbackMethod_Listen() {
      ::grpc::Service::MarkMethodRawCallback(0,
          new ::grpc::internal::CallbackServerStreamingHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
                   ::grpc::CallbackServerContext* context, const::grpc::ByteBuffer* request) { return this->Listen(context, request); }));
    }
    ~WithRawCallbackMethod_Listen() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Listen(::grpc::ServerContext* /*context*/, const ::ChlorobotAuthentication* /*request*/, ::grpc::ServerWriter< ::ChlorobotPacket>* /*writer*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerWriteReactor< ::grpc::ByteBuffer>* Listen(
      ::grpc::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithRawCallbackMethod_Send : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawCallbackMethod_Send() {
      ::grpc::Service::MarkMethodRawCallback(1,
          new ::grpc::internal::CallbackUnaryHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::grpc::ByteBuffer* request, ::grpc::ByteBuffer* response) { return this->Send(context, request, response); }));
    }
    ~WithRawCallbackMethod_Send() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Send(::grpc::ServerContext* /*context*/, const ::ChlorobotRequest* /*request*/, ::ChlorobotAcknowledgement* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* Send(
      ::grpc::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/, ::grpc::ByteBuffer* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_Send : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithStreamedUnaryMethod_Send() {
      ::grpc::Service::MarkMethodStreamed(1,
        new ::grpc::internal::StreamedUnaryHandler<
          ::ChlorobotRequest, ::ChlorobotAcknowledgement>(
            [this](::grpc::ServerContext* context,
                   ::grpc::ServerUnaryStreamer<
                     ::ChlorobotRequest, ::ChlorobotAcknowledgement>* streamer) {
                       return this->StreamedSend(context,
                         streamer);
                  }));
    }
    ~WithStreamedUnaryMethod_Send() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status Send(::grpc::ServerContext* /*context*/, const ::ChlorobotRequest* /*request*/, ::ChlorobotAcknowledgement* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedSend(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::ChlorobotRequest,::ChlorobotAcknowledgement>* server_unary_streamer) = 0;
  };
  typedef WithStreamedUnaryMethod_Send<Service > StreamedUnaryService;
  template <class BaseClass>
  class WithSplitStreamingMethod_Listen : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithSplitStreamingMethod_Listen() {
      ::grpc::Service::MarkMethodStreamed(0,
        new ::grpc::internal::SplitServerStreamingHandler<
          ::ChlorobotAuthentication, ::ChlorobotPacket>(
            [this](::grpc::ServerContext* context,
                   ::grpc::ServerSplitStreamer<
                     ::ChlorobotAuthentication, ::ChlorobotPacket>* streamer) {
                       return this->StreamedListen(context,
                         streamer);
                  }));
    }
    ~WithSplitStreamingMethod_Listen() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status Listen(::grpc::ServerContext* /*context*/, const ::ChlorobotAuthentication* /*request*/, ::grpc::ServerWriter< ::ChlorobotPacket>* /*writer*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with split streamed
    virtual ::grpc::Status StreamedListen(::grpc::ServerContext* context, ::grpc::ServerSplitStreamer< ::ChlorobotAuthentication,::ChlorobotPacket>* server_split_streamer) = 0;
  };
  typedef WithSplitStreamingMethod_Listen<Service > SplitStreamedService;
  typedef WithSplitStreamingMethod_Listen<WithStreamedUnaryMethod_Send<Service > > StreamedService;
};


#endif  // GRPC_chlorobot_5frpc_2eproto__INCLUDED
