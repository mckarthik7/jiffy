/**
 * Autogenerated by Thrift Compiler (0.11.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef chain_response_service_H
#define chain_response_service_H

#include <thrift/TDispatchProcessor.h>
#include <thrift/async/TConcurrentClientSyncInfo.h>
#include "block_service_types.h"

namespace jiffy { namespace storage {

#ifdef _MSC_VER
  #pragma warning( push )
  #pragma warning (disable : 4250 ) //inheriting methods via dominance 
#endif

class chain_response_serviceIf {
 public:
  virtual ~chain_response_serviceIf() {}
  virtual void chain_ack(const sequence_id& seq) = 0;
};

class chain_response_serviceIfFactory {
 public:
  typedef chain_response_serviceIf Handler;

  virtual ~chain_response_serviceIfFactory() {}

  virtual chain_response_serviceIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(chain_response_serviceIf* /* handler */) = 0;
};

class chain_response_serviceIfSingletonFactory : virtual public chain_response_serviceIfFactory {
 public:
  chain_response_serviceIfSingletonFactory(const ::apache::thrift::stdcxx::shared_ptr<chain_response_serviceIf>& iface) : iface_(iface) {}
  virtual ~chain_response_serviceIfSingletonFactory() {}

  virtual chain_response_serviceIf* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(chain_response_serviceIf* /* handler */) {}

 protected:
  ::apache::thrift::stdcxx::shared_ptr<chain_response_serviceIf> iface_;
};

class chain_response_serviceNull : virtual public chain_response_serviceIf {
 public:
  virtual ~chain_response_serviceNull() {}
  void chain_ack(const sequence_id& /* seq */) {
    return;
  }
};

typedef struct _chain_response_service_chain_ack_args__isset {
  _chain_response_service_chain_ack_args__isset() : seq(false) {}
  bool seq :1;
} _chain_response_service_chain_ack_args__isset;

class chain_response_service_chain_ack_args {
 public:

  chain_response_service_chain_ack_args(const chain_response_service_chain_ack_args&);
  chain_response_service_chain_ack_args& operator=(const chain_response_service_chain_ack_args&);
  chain_response_service_chain_ack_args() {
  }

  virtual ~chain_response_service_chain_ack_args() throw();
  sequence_id seq;

  _chain_response_service_chain_ack_args__isset __isset;

  void __set_seq(const sequence_id& val);

  bool operator == (const chain_response_service_chain_ack_args & rhs) const
  {
    if (!(seq == rhs.seq))
      return false;
    return true;
  }
  bool operator != (const chain_response_service_chain_ack_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const chain_response_service_chain_ack_args & ) const;

  template <class Protocol_>
  uint32_t read(Protocol_* iprot);
  template <class Protocol_>
  uint32_t write(Protocol_* oprot) const;

};


class chain_response_service_chain_ack_pargs {
 public:


  virtual ~chain_response_service_chain_ack_pargs() throw();
  const sequence_id* seq;

  template <class Protocol_>
  uint32_t write(Protocol_* oprot) const;

};

template <class Protocol_>
class chain_response_serviceClientT : virtual public chain_response_serviceIf {
 public:
  chain_response_serviceClientT(apache::thrift::stdcxx::shared_ptr< Protocol_> prot) {
    setProtocolT(prot);
  }
  chain_response_serviceClientT(apache::thrift::stdcxx::shared_ptr< Protocol_> iprot, apache::thrift::stdcxx::shared_ptr< Protocol_> oprot) {
    setProtocolT(iprot,oprot);
  }
 private:
  void setProtocolT(apache::thrift::stdcxx::shared_ptr< Protocol_> prot) {
  setProtocolT(prot,prot);
  }
  void setProtocolT(apache::thrift::stdcxx::shared_ptr< Protocol_> iprot, apache::thrift::stdcxx::shared_ptr< Protocol_> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return this->piprot_;
  }
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return this->poprot_;
  }
  void chain_ack(const sequence_id& seq);
  void send_chain_ack(const sequence_id& seq);
 protected:
  apache::thrift::stdcxx::shared_ptr< Protocol_> piprot_;
  apache::thrift::stdcxx::shared_ptr< Protocol_> poprot_;
  Protocol_* iprot_;
  Protocol_* oprot_;
};

typedef chain_response_serviceClientT< ::apache::thrift::protocol::TProtocol> chain_response_serviceClient;

template <class Protocol_>
class chain_response_serviceProcessorT : public ::apache::thrift::TDispatchProcessorT<Protocol_> {
 protected:
  ::apache::thrift::stdcxx::shared_ptr<chain_response_serviceIf> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
  virtual bool dispatchCallTemplated(Protocol_* iprot, Protocol_* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (chain_response_serviceProcessorT::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef void (chain_response_serviceProcessorT::*SpecializedProcessFunction)(int32_t, Protocol_*, Protocol_*, void*);
  struct ProcessFunctions {
    ProcessFunction generic;
    SpecializedProcessFunction specialized;
    ProcessFunctions(ProcessFunction g, SpecializedProcessFunction s) :
      generic(g),
      specialized(s) {}
    ProcessFunctions() : generic(NULL), specialized(NULL) {}
  };
  typedef std::map<std::string, ProcessFunctions> ProcessMap;
  ProcessMap processMap_;
  void process_chain_ack(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_chain_ack(int32_t seqid, Protocol_* iprot, Protocol_* oprot, void* callContext);
 public:
  chain_response_serviceProcessorT(::apache::thrift::stdcxx::shared_ptr<chain_response_serviceIf> iface) :
    iface_(iface) {
    processMap_["chain_ack"] = ProcessFunctions(
      &chain_response_serviceProcessorT::process_chain_ack,
      &chain_response_serviceProcessorT::process_chain_ack);
  }

  virtual ~chain_response_serviceProcessorT() {}
};

typedef chain_response_serviceProcessorT< ::apache::thrift::protocol::TDummyProtocol > chain_response_serviceProcessor;

template <class Protocol_>
class chain_response_serviceProcessorFactoryT : public ::apache::thrift::TProcessorFactory {
 public:
  chain_response_serviceProcessorFactoryT(const ::apache::thrift::stdcxx::shared_ptr< chain_response_serviceIfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::apache::thrift::stdcxx::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::apache::thrift::stdcxx::shared_ptr< chain_response_serviceIfFactory > handlerFactory_;
};

typedef chain_response_serviceProcessorFactoryT< ::apache::thrift::protocol::TDummyProtocol > chain_response_serviceProcessorFactory;

class chain_response_serviceMultiface : virtual public chain_response_serviceIf {
 public:
  chain_response_serviceMultiface(std::vector<apache::thrift::stdcxx::shared_ptr<chain_response_serviceIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~chain_response_serviceMultiface() {}
 protected:
  std::vector<apache::thrift::stdcxx::shared_ptr<chain_response_serviceIf> > ifaces_;
  chain_response_serviceMultiface() {}
  void add(::apache::thrift::stdcxx::shared_ptr<chain_response_serviceIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void chain_ack(const sequence_id& seq) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->chain_ack(seq);
    }
    ifaces_[i]->chain_ack(seq);
  }

};

// The 'concurrent' client is a thread safe client that correctly handles
// out of order responses.  It is slower than the regular client, so should
// only be used when you need to share a connection among multiple threads
template <class Protocol_>
class chain_response_serviceConcurrentClientT : virtual public chain_response_serviceIf {
 public:
  chain_response_serviceConcurrentClientT(apache::thrift::stdcxx::shared_ptr< Protocol_> prot) {
    setProtocolT(prot);
  }
  chain_response_serviceConcurrentClientT(apache::thrift::stdcxx::shared_ptr< Protocol_> iprot, apache::thrift::stdcxx::shared_ptr< Protocol_> oprot) {
    setProtocolT(iprot,oprot);
  }
 private:
  void setProtocolT(apache::thrift::stdcxx::shared_ptr< Protocol_> prot) {
  setProtocolT(prot,prot);
  }
  void setProtocolT(apache::thrift::stdcxx::shared_ptr< Protocol_> iprot, apache::thrift::stdcxx::shared_ptr< Protocol_> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return this->piprot_;
  }
  apache::thrift::stdcxx::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return this->poprot_;
  }
  void chain_ack(const sequence_id& seq);
  void send_chain_ack(const sequence_id& seq);
 protected:
  apache::thrift::stdcxx::shared_ptr< Protocol_> piprot_;
  apache::thrift::stdcxx::shared_ptr< Protocol_> poprot_;
  Protocol_* iprot_;
  Protocol_* oprot_;
  ::apache::thrift::async::TConcurrentClientSyncInfo sync_;
};

typedef chain_response_serviceConcurrentClientT< ::apache::thrift::protocol::TProtocol> chain_response_serviceConcurrentClient;

#ifdef _MSC_VER
  #pragma warning( pop )
#endif

}} // namespace

#include "chain_response_service.tcc"
#include "block_service_types.tcc"

#endif