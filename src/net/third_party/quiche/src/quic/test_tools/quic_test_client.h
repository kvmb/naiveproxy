// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QUICHE_QUIC_TEST_TOOLS_QUIC_TEST_CLIENT_H_
#define QUICHE_QUIC_TEST_TOOLS_QUIC_TEST_CLIENT_H_

#include <cstdint>
#include <memory>
#include <string>

#include "absl/strings/string_view.h"
#include "quic/core/proto/cached_network_parameters_proto.h"
#include "quic/core/quic_framer.h"
#include "quic/core/quic_packet_creator.h"
#include "quic/core/quic_packets.h"
#include "quic/platform/api/quic_epoll.h"
#include "quic/platform/api/quic_test.h"
#include "quic/tools/quic_client.h"
#include "common/quiche_linked_hash_map.h"

namespace quic {

class ProofVerifier;
class QuicPacketWriterWrapper;

namespace test {

class MockableQuicClientEpollNetworkHelper;

// A quic client which allows mocking out reads and writes.
class MockableQuicClient : public QuicClient {
 public:
  MockableQuicClient(QuicSocketAddress server_address,
                     const QuicServerId& server_id,
                     const ParsedQuicVersionVector& supported_versions,
                     QuicEpollServer* epoll_server);

  MockableQuicClient(QuicSocketAddress server_address,
                     const QuicServerId& server_id,
                     const QuicConfig& config,
                     const ParsedQuicVersionVector& supported_versions,
                     QuicEpollServer* epoll_server);

  MockableQuicClient(QuicSocketAddress server_address,
                     const QuicServerId& server_id,
                     const QuicConfig& config,
                     const ParsedQuicVersionVector& supported_versions,
                     QuicEpollServer* epoll_server,
                     std::unique_ptr<ProofVerifier> proof_verifier);

  MockableQuicClient(QuicSocketAddress server_address,
                     const QuicServerId& server_id,
                     const QuicConfig& config,
                     const ParsedQuicVersionVector& supported_versions,
                     QuicEpollServer* epoll_server,
                     std::unique_ptr<ProofVerifier> proof_verifier,
                     std::unique_ptr<SessionCache> session_cache);
  MockableQuicClient(const MockableQuicClient&) = delete;
  MockableQuicClient& operator=(const MockableQuicClient&) = delete;

  ~MockableQuicClient() override;

  QuicConnectionId GenerateNewConnectionId() override;
  void UseConnectionId(QuicConnectionId server_connection_id);
  void UseConnectionIdLength(int server_connection_id_length);
  QuicConnectionId GetClientConnectionId() override;
  void UseClientConnectionId(QuicConnectionId client_connection_id);
  void UseClientConnectionIdLength(int client_connection_id_length);

  void UseWriter(QuicPacketWriterWrapper* writer);
  void set_peer_address(const QuicSocketAddress& address);
  // The last incoming packet, iff |track_last_incoming_packet| is true.
  const QuicReceivedPacket* last_incoming_packet();
  // If true, copy each packet from ProcessPacket into |last_incoming_packet|
  void set_track_last_incoming_packet(bool track);

  // Casts the network helper to a MockableQuicClientEpollNetworkHelper.
  MockableQuicClientEpollNetworkHelper* mockable_network_helper();
  const MockableQuicClientEpollNetworkHelper* mockable_network_helper() const;

 private:
  // Server connection ID to use, if server_connection_id_overridden_
  QuicConnectionId override_server_connection_id_;
  bool server_connection_id_overridden_;
  int override_server_connection_id_length_ = -1;
  // Client connection ID to use, if client_connection_id_overridden_
  QuicConnectionId override_client_connection_id_;
  bool client_connection_id_overridden_;
  int override_client_connection_id_length_ = -1;
  CachedNetworkParameters cached_network_paramaters_;
};

// A toy QUIC client used for testing.
class QuicTestClient : public QuicSpdyStream::Visitor,
                       public QuicClientPushPromiseIndex::Delegate {
 public:
  QuicTestClient(QuicSocketAddress server_address,
                 const std::string& server_hostname,
                 const ParsedQuicVersionVector& supported_versions);
  QuicTestClient(QuicSocketAddress server_address,
                 const std::string& server_hostname,
                 const QuicConfig& config,
                 const ParsedQuicVersionVector& supported_versions);
  QuicTestClient(QuicSocketAddress server_address,
                 const std::string& server_hostname,
                 const QuicConfig& config,
                 const ParsedQuicVersionVector& supported_versions,
                 std::unique_ptr<ProofVerifier> proof_verifier);
  QuicTestClient(QuicSocketAddress server_address,
                 const std::string& server_hostname,
                 const QuicConfig& config,
                 const ParsedQuicVersionVector& supported_versions,
                 std::unique_ptr<ProofVerifier> proof_verifier,
                 std::unique_ptr<SessionCache> session_cache);

  ~QuicTestClient() override;

  // Sets the |user_agent_id| of the |client_|.
  void SetUserAgentID(const std::string& user_agent_id);

  // Wraps data in a quic packet and sends it.
  ssize_t SendData(const std::string& data, bool last_data);
  // As above, but |delegate| will be notified when |data| is ACKed.
  ssize_t SendData(
      const std::string& data,
      bool last_data,
      QuicReferenceCountedPointer<QuicAckListenerInterface> ack_listener);

  // Clears any outstanding state and sends a simple GET of 'uri' to the
  // server.  Returns 0 if the request failed and no bytes were written.
  ssize_t SendRequest(const std::string& uri);
  // Send a request R and a RST_FRAME which resets R, in the same packet.
  ssize_t SendRequestAndRstTogether(const std::string& uri);
  // Sends requests for all the urls and waits for the responses.  To process
  // the individual responses as they are returned, the caller should use the
  // set the response_listener on the client().
  void SendRequestsAndWaitForResponses(
      const std::vector<std::string>& url_list);
  // Sends a request containing |headers| and |body| and returns the number of
  // bytes sent (the size of the serialized request headers and body).
  ssize_t SendMessage(const spdy::SpdyHeaderBlock& headers,
                      absl::string_view body);
  // Sends a request containing |headers| and |body| with the fin bit set to
  // |fin| and returns the number of bytes sent (the size of the serialized
  // request headers and body).
  ssize_t SendMessage(const spdy::SpdyHeaderBlock& headers,
                      absl::string_view body,
                      bool fin);
  // Sends a request containing |headers| and |body| with the fin bit set to
  // |fin| and returns the number of bytes sent (the size of the serialized
  // request headers and body). If |flush| is true, will wait for the message to
  // be flushed before returning.
  ssize_t SendMessage(const spdy::SpdyHeaderBlock& headers,
                      absl::string_view body,
                      bool fin,
                      bool flush);
  // Sends a request containing |headers| and |body|, waits for the response,
  // and returns the response body.
  std::string SendCustomSynchronousRequest(const spdy::SpdyHeaderBlock& headers,
                                           const std::string& body);
  // Sends a GET request for |uri|, waits for the response, and returns the
  // response body.
  std::string SendSynchronousRequest(const std::string& uri);
  void SendConnectivityProbing();
  void Connect();
  void ResetConnection();
  void Disconnect();
  QuicSocketAddress local_address() const;
  void ClearPerRequestState();
  bool WaitUntil(int timeout_ms, std::function<bool()> trigger);
  ssize_t Send(absl::string_view data);
  bool connected() const;
  bool buffer_body() const;
  void set_buffer_body(bool buffer_body);

  // Getters for stream state. Please note, these getters are divided into two
  // groups. 1) returns state which only get updated once a complete response
  // is received. 2) returns state of the oldest active stream which have
  // received partial response (if any).
  // Group 1.
  const spdy::SpdyHeaderBlock& response_trailers() const;
  bool response_complete() const;
  int64_t response_body_size() const;
  const std::string& response_body() const;
  // Group 2.
  bool response_headers_complete() const;
  const spdy::SpdyHeaderBlock* response_headers() const;
  const spdy::SpdyHeaderBlock* preliminary_headers() const;
  int64_t response_size() const;
  size_t bytes_read() const;
  size_t bytes_written() const;

  // Returns once at least one complete response or a connection close has been
  // received from the server. If responses are received for multiple (say 2)
  // streams, next WaitForResponse will return immediately.
  void WaitForResponse() { WaitForResponseForMs(-1); }

  // Returns once some data is received on any open streams or at least one
  // complete response is received from the server.
  void WaitForInitialResponse() { WaitForInitialResponseForMs(-1); }

  // Returns once at least one complete response or a connection close has been
  // received from the server, or once the timeout expires.
  // Passing in a timeout value of -1 disables the timeout. If multiple
  // responses are received while the client is waiting, subsequent calls to
  // this function will return immediately.
  void WaitForResponseForMs(int timeout_ms) {
    WaitUntil(timeout_ms, [this]() { return !closed_stream_states_.empty(); });
    if (response_complete()) {
      QUIC_VLOG(1) << "Client received response:"
                   << response_headers()->DebugString() << response_body();
    }
  }

  // Returns once some data is received on any open streams or at least one
  // complete response is received from the server, or once the timeout
  // expires. -1 means no timeout.
  void WaitForInitialResponseForMs(int timeout_ms) {
    WaitUntil(timeout_ms, [this]() { return response_size() != 0; });
  }

  // Migrate local address to <|new_host|, a random port>.
  // Return whether the migration succeeded.
  bool MigrateSocket(const QuicIpAddress& new_host);
  // Migrate local address to <|new_host|, |port|>.
  // Return whether the migration succeeded.
  bool MigrateSocketWithSpecifiedPort(const QuicIpAddress& new_host, int port);
  QuicIpAddress bind_to_address() const;
  void set_bind_to_address(QuicIpAddress address);
  const QuicSocketAddress& address() const;

  // From QuicSpdyStream::Visitor
  void OnClose(QuicSpdyStream* stream) override;

  // From QuicClientPushPromiseIndex::Delegate
  bool CheckVary(const spdy::SpdyHeaderBlock& client_request,
                 const spdy::SpdyHeaderBlock& promise_request,
                 const spdy::SpdyHeaderBlock& promise_response) override;
  void OnRendezvousResult(QuicSpdyStream*) override;

  // Configures client_ to take ownership of and use the writer.
  // Must be called before initial connect.
  void UseWriter(QuicPacketWriterWrapper* writer);
  // Configures client_ to use a specific server connection ID instead of a
  // random one.
  void UseConnectionId(QuicConnectionId server_connection_id);
  // Configures client_ to use a specific server connection ID length instead
  // of the default of kQuicDefaultConnectionIdLength.
  void UseConnectionIdLength(int server_connection_id_length);
  // Configures client_ to use a specific client connection ID instead of an
  // empty one.
  void UseClientConnectionId(QuicConnectionId client_connection_id);
  // Configures client_ to use a specific client connection ID length instead
  // of the default of zero.
  void UseClientConnectionIdLength(int client_connection_id_length);

  // Returns nullptr if the maximum number of streams have already been created.
  QuicSpdyClientStream* GetOrCreateStream();

  // Calls GetOrCreateStream(), sends the request on the stream, and
  // stores the request in case it needs to be resent.  If |headers| is
  // null, only the body will be sent on the stream.
  ssize_t GetOrCreateStreamAndSendRequest(
      const spdy::SpdyHeaderBlock* headers,
      absl::string_view body,
      bool fin,
      QuicReferenceCountedPointer<QuicAckListenerInterface> ack_listener);

  QuicRstStreamErrorCode stream_error() { return stream_error_; }
  QuicErrorCode connection_error() const;

  MockableQuicClient* client() { return client_.get(); }
  const MockableQuicClient* client() const { return client_.get(); }

  // cert_common_name returns the common name value of the server's certificate,
  // or the empty std::string if no certificate was presented.
  const std::string& cert_common_name() const;

  // cert_sct returns the signed timestamp of the server's certificate,
  // or the empty std::string if no signed timestamp was presented.
  const std::string& cert_sct() const;

  // Get the server config map.  Server config must exist.
  const QuicTagValueMap& GetServerConfig() const;

  void set_auto_reconnect(bool reconnect) { auto_reconnect_ = reconnect; }

  void set_priority(spdy::SpdyPriority priority) { priority_ = priority; }

  void WaitForWriteToFlush();

  QuicEpollServer* epoll_server() { return &epoll_server_; }

  size_t num_requests() const { return num_requests_; }

  size_t num_responses() const { return num_responses_; }

  void set_server_address(const QuicSocketAddress& server_address) {
    client_->set_server_address(server_address);
  }

  void set_peer_address(const QuicSocketAddress& address) {
    client_->set_peer_address(address);
  }

  // Explicitly set the SNI value for this client, overriding the default
  // behavior which extracts the SNI value from the request URL.
  void OverrideSni(const std::string& sni) {
    override_sni_set_ = true;
    override_sni_ = sni;
  }

  void Initialize();

  void set_client(MockableQuicClient* client) { client_.reset(client); }

  // Given |uri|, populates the fields in |headers| for a simple GET
  // request. If |uri| is a relative URL, the QuicServerId will be
  // use to specify the authority.
  bool PopulateHeaderBlockFromUrl(const std::string& uri,
                                  spdy::SpdyHeaderBlock* headers);

  // Waits for a period of time that is long enough to receive all delayed acks
  // sent by peer.
  void WaitForDelayedAcks();

  QuicSpdyClientStream* latest_created_stream() {
    return latest_created_stream_;
  }

 protected:
  QuicTestClient();
  QuicTestClient(const QuicTestClient&) = delete;
  QuicTestClient(const QuicTestClient&&) = delete;
  QuicTestClient& operator=(const QuicTestClient&) = delete;
  QuicTestClient& operator=(const QuicTestClient&&) = delete;

 private:
  class TestClientDataToResend : public QuicClient::QuicDataToResend {
   public:
    TestClientDataToResend(
        std::unique_ptr<spdy::SpdyHeaderBlock> headers,
        absl::string_view body,
        bool fin,
        QuicTestClient* test_client,
        QuicReferenceCountedPointer<QuicAckListenerInterface> ack_listener);

    ~TestClientDataToResend() override;

    void Resend() override;

   protected:
    QuicTestClient* test_client_;
    QuicReferenceCountedPointer<QuicAckListenerInterface> ack_listener_;
  };

  // PerStreamState of a stream is updated when it is closed.
  struct PerStreamState {
    PerStreamState(const PerStreamState& other);
    PerStreamState(QuicRstStreamErrorCode stream_error,
                   bool response_complete,
                   bool response_headers_complete,
                   const spdy::SpdyHeaderBlock& response_headers,
                   const spdy::SpdyHeaderBlock& preliminary_headers,
                   const std::string& response,
                   const spdy::SpdyHeaderBlock& response_trailers,
                   uint64_t bytes_read,
                   uint64_t bytes_written,
                   int64_t response_body_size);
    ~PerStreamState();

    QuicRstStreamErrorCode stream_error;
    bool response_complete;
    bool response_headers_complete;
    spdy::SpdyHeaderBlock response_headers;
    spdy::SpdyHeaderBlock preliminary_headers;
    std::string response;
    spdy::SpdyHeaderBlock response_trailers;
    uint64_t bytes_read;
    uint64_t bytes_written;
    int64_t response_body_size;
  };

  bool HaveActiveStream();

  // Read oldest received response and remove it from closed_stream_states_.
  void ReadNextResponse();

  // Clear open_streams_, closed_stream_states_ and reset
  // latest_created_stream_.
  void ClearPerConnectionState();

  // Update latest_created_stream_, add |stream| to open_streams_ and starts
  // tracking its state.
  void SetLatestCreatedStream(QuicSpdyClientStream* stream);

  QuicEpollServer epoll_server_;
  std::unique_ptr<MockableQuicClient> client_;  // The actual client
  QuicSpdyClientStream* latest_created_stream_;
  std::map<QuicStreamId, QuicSpdyClientStream*> open_streams_;
  // Received responses of closed streams.
  quiche::QuicheLinkedHashMap<QuicStreamId, PerStreamState>
      closed_stream_states_;

  QuicRstStreamErrorCode stream_error_;

  bool response_complete_;
  bool response_headers_complete_;
  mutable spdy::SpdyHeaderBlock preliminary_headers_;
  mutable spdy::SpdyHeaderBlock response_headers_;

  // Parsed response trailers (if present), copied from the stream in OnClose.
  spdy::SpdyHeaderBlock response_trailers_;

  spdy::SpdyPriority priority_;
  std::string response_;
  // bytes_read_ and bytes_written_ are updated only when stream_ is released;
  // prefer bytes_read() and bytes_written() member functions.
  uint64_t bytes_read_;
  uint64_t bytes_written_;
  // The number of HTTP body bytes received.
  int64_t response_body_size_;
  // True if we tried to connect already since the last call to Disconnect().
  bool connect_attempted_;
  // The client will auto-connect exactly once before sending data.  If
  // something causes a connection reset, it will not automatically reconnect
  // unless auto_reconnect_ is true.
  bool auto_reconnect_;
  // Should we buffer the response body? Defaults to true.
  bool buffer_body_;
  // For async push promise rendezvous, validation may fail in which
  // case the request should be retried.
  std::unique_ptr<TestClientDataToResend> push_promise_data_to_resend_;
  // Number of requests/responses this client has sent/received.
  size_t num_requests_;
  size_t num_responses_;

  // If set, this value is used for the connection SNI, overriding the usual
  // logic which extracts the SNI from the request URL.
  bool override_sni_set_ = false;
  std::string override_sni_;
};

}  // namespace test

}  // namespace quic

#endif  // QUICHE_QUIC_TEST_TOOLS_QUIC_TEST_CLIENT_H_
