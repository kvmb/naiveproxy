// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "quic/test_tools/quic_connection_peer.h"

#include "absl/strings/string_view.h"
#include "quic/core/congestion_control/send_algorithm_interface.h"
#include "quic/core/quic_packet_writer.h"
#include "quic/core/quic_received_packet_manager.h"
#include "quic/platform/api/quic_flags.h"
#include "quic/platform/api/quic_socket_address.h"
#include "quic/test_tools/quic_connection_id_manager_peer.h"
#include "quic/test_tools/quic_framer_peer.h"
#include "quic/test_tools/quic_sent_packet_manager_peer.h"

namespace quic {
namespace test {

// static
void QuicConnectionPeer::SetSendAlgorithm(
    QuicConnection* connection,
    SendAlgorithmInterface* send_algorithm) {
  GetSentPacketManager(connection)->SetSendAlgorithm(send_algorithm);
}

// static
void QuicConnectionPeer::SetLossAlgorithm(
    QuicConnection* connection,
    LossDetectionInterface* loss_algorithm) {
  GetSentPacketManager(connection)->loss_algorithm_ = loss_algorithm;
}

// static
void QuicConnectionPeer::PopulateStopWaitingFrame(
    QuicConnection* connection,
    QuicStopWaitingFrame* stop_waiting) {
  connection->PopulateStopWaitingFrame(stop_waiting);
}

// static
QuicPacketCreator* QuicConnectionPeer::GetPacketCreator(
    QuicConnection* connection) {
  return &connection->packet_creator_;
}

// static
QuicSentPacketManager* QuicConnectionPeer::GetSentPacketManager(
    QuicConnection* connection) {
  return &connection->sent_packet_manager_;
}

// static
QuicTime::Delta QuicConnectionPeer::GetNetworkTimeout(
    QuicConnection* connection) {
  return connection->idle_network_detector_.idle_network_timeout_;
}

// static
QuicTime::Delta QuicConnectionPeer::GetHandshakeTimeout(
    QuicConnection* connection) {
  return connection->idle_network_detector_.handshake_timeout_;
}

// static
void QuicConnectionPeer::SetPerspective(QuicConnection* connection,
                                        Perspective perspective) {
  connection->perspective_ = perspective;
  QuicFramerPeer::SetPerspective(&connection->framer_, perspective);
}

// static
void QuicConnectionPeer::SetSelfAddress(QuicConnection* connection,
                                        const QuicSocketAddress& self_address) {
  connection->default_path_.self_address = self_address;
}

// static
void QuicConnectionPeer::SetPeerAddress(QuicConnection* connection,
                                        const QuicSocketAddress& peer_address) {
  connection->UpdatePeerAddress(peer_address);
}

// static
void QuicConnectionPeer::SetDirectPeerAddress(
    QuicConnection* connection,
    const QuicSocketAddress& direct_peer_address) {
  connection->direct_peer_address_ = direct_peer_address;
}

// static
void QuicConnectionPeer::SetEffectivePeerAddress(
    QuicConnection* connection,
    const QuicSocketAddress& effective_peer_address) {
  connection->default_path_.peer_address = effective_peer_address;
}

// static
void QuicConnectionPeer::SwapCrypters(QuicConnection* connection,
                                      QuicFramer* framer) {
  QuicFramerPeer::SwapCrypters(framer, &connection->framer_);
}

// static
void QuicConnectionPeer::SetCurrentPacket(QuicConnection* connection,
                                          absl::string_view current_packet) {
  connection->current_packet_data_ = current_packet.data();
  connection->last_size_ = current_packet.size();
}

// static
QuicConnectionHelperInterface* QuicConnectionPeer::GetHelper(
    QuicConnection* connection) {
  return connection->helper_;
}

// static
QuicAlarmFactory* QuicConnectionPeer::GetAlarmFactory(
    QuicConnection* connection) {
  return connection->alarm_factory_;
}

// static
QuicFramer* QuicConnectionPeer::GetFramer(QuicConnection* connection) {
  return &connection->framer_;
}

// static
QuicAlarm* QuicConnectionPeer::GetAckAlarm(QuicConnection* connection) {
  return connection->ack_alarm_.get();
}

// static
QuicAlarm* QuicConnectionPeer::GetPingAlarm(QuicConnection* connection) {
  return connection->ping_alarm_.get();
}

// static
QuicAlarm* QuicConnectionPeer::GetRetransmissionAlarm(
    QuicConnection* connection) {
  return connection->retransmission_alarm_.get();
}

// static
QuicAlarm* QuicConnectionPeer::GetSendAlarm(QuicConnection* connection) {
  return connection->send_alarm_.get();
}

// static
QuicAlarm* QuicConnectionPeer::GetMtuDiscoveryAlarm(
    QuicConnection* connection) {
  return connection->mtu_discovery_alarm_.get();
}

// static
QuicAlarm* QuicConnectionPeer::GetProcessUndecryptablePacketsAlarm(
    QuicConnection* connection) {
  return connection->process_undecryptable_packets_alarm_.get();
}

// static
QuicAlarm* QuicConnectionPeer::GetDiscardPreviousOneRttKeysAlarm(
    QuicConnection* connection) {
  return connection->discard_previous_one_rtt_keys_alarm_.get();
}

// static
QuicAlarm* QuicConnectionPeer::GetDiscardZeroRttDecryptionKeysAlarm(
    QuicConnection* connection) {
  return connection->discard_zero_rtt_decryption_keys_alarm_.get();
}

// static
QuicAlarm* QuicConnectionPeer::GetRetirePeerIssuedConnectionIdAlarm(
    QuicConnection* connection) {
  if (connection->peer_issued_cid_manager_ == nullptr) {
    return nullptr;
  }
  return QuicConnectionIdManagerPeer::GetRetirePeerIssuedConnectionIdAlarm(
      connection->peer_issued_cid_manager_.get());
}
// static
QuicAlarm* QuicConnectionPeer::GetRetireSelfIssuedConnectionIdAlarm(
    QuicConnection* connection) {
  if (connection->self_issued_cid_manager_ == nullptr) {
    return nullptr;
  }
  return QuicConnectionIdManagerPeer::GetRetireSelfIssuedConnectionIdAlarm(
      connection->self_issued_cid_manager_.get());
}

// static
QuicPacketWriter* QuicConnectionPeer::GetWriter(QuicConnection* connection) {
  return connection->writer_;
}

// static
void QuicConnectionPeer::SetWriter(QuicConnection* connection,
                                   QuicPacketWriter* writer,
                                   bool owns_writer) {
  if (connection->owns_writer_) {
    delete connection->writer_;
  }
  connection->writer_ = writer;
  connection->owns_writer_ = owns_writer;
}

// static
void QuicConnectionPeer::TearDownLocalConnectionState(
    QuicConnection* connection) {
  connection->connected_ = false;
}

// static
QuicEncryptedPacket* QuicConnectionPeer::GetConnectionClosePacket(
    QuicConnection* connection) {
  if (connection->termination_packets_ == nullptr ||
      connection->termination_packets_->empty()) {
    return nullptr;
  }
  return (*connection->termination_packets_)[0].get();
}

// static
QuicPacketHeader* QuicConnectionPeer::GetLastHeader(
    QuicConnection* connection) {
  return &connection->last_header_;
}

// static
QuicConnectionStats* QuicConnectionPeer::GetStats(QuicConnection* connection) {
  return &connection->stats_;
}

// static
QuicPacketCount QuicConnectionPeer::GetPacketsBetweenMtuProbes(
    QuicConnection* connection) {
  return connection->mtu_discoverer_.packets_between_probes();
}

// static
void QuicConnectionPeer::ReInitializeMtuDiscoverer(
    QuicConnection* connection,
    QuicPacketCount packets_between_probes_base,
    QuicPacketNumber next_probe_at) {
  connection->mtu_discoverer_ =
      QuicConnectionMtuDiscoverer(packets_between_probes_base, next_probe_at);
}

// static
void QuicConnectionPeer::SetAckDecimationDelay(QuicConnection* connection,
                                               float ack_decimation_delay) {
  for (auto& received_packet_manager :
       connection->uber_received_packet_manager_.received_packet_managers_) {
    received_packet_manager.ack_decimation_delay_ = ack_decimation_delay;
  }
}

// static
bool QuicConnectionPeer::HasRetransmittableFrames(QuicConnection* connection,
                                                  uint64_t packet_number) {
  return QuicSentPacketManagerPeer::HasRetransmittableFrames(
      GetSentPacketManager(connection), packet_number);
}

// static
bool QuicConnectionPeer::GetNoStopWaitingFrames(QuicConnection* connection) {
  return connection->no_stop_waiting_frames_;
}

// static
void QuicConnectionPeer::SetNoStopWaitingFrames(QuicConnection* connection,
                                                bool no_stop_waiting_frames) {
  connection->no_stop_waiting_frames_ = no_stop_waiting_frames;
}

// static
void QuicConnectionPeer::SetMaxTrackedPackets(
    QuicConnection* connection,
    QuicPacketCount max_tracked_packets) {
  connection->max_tracked_packets_ = max_tracked_packets;
}

// static
void QuicConnectionPeer::SetNegotiatedVersion(QuicConnection* connection) {
  connection->version_negotiated_ = true;
  if (connection->perspective() == Perspective::IS_SERVER &&
      !QuicFramerPeer::infer_packet_header_type_from_version(
          &connection->framer_)) {
    connection->framer_.InferPacketHeaderTypeFromVersion();
  }
}

// static
void QuicConnectionPeer::SetMaxConsecutiveNumPacketsWithNoRetransmittableFrames(
    QuicConnection* connection,
    size_t new_value) {
  connection->max_consecutive_num_packets_with_no_retransmittable_frames_ =
      new_value;
}

// static
bool QuicConnectionPeer::SupportsReleaseTime(QuicConnection* connection) {
  return connection->supports_release_time_;
}

// static
QuicConnection::PacketContent QuicConnectionPeer::GetCurrentPacketContent(
    QuicConnection* connection) {
  return connection->current_packet_content_;
}

// static
void QuicConnectionPeer::SetLastHeaderFormat(QuicConnection* connection,
                                             PacketHeaderFormat format) {
  connection->last_header_.form = format;
}

// static
void QuicConnectionPeer::AddBytesReceived(QuicConnection* connection,
                                          size_t length) {
  if (connection->EnforceAntiAmplificationLimit()) {
    connection->default_path_.bytes_received_before_address_validation +=
        length;
  }
}

// static
void QuicConnectionPeer::SetAddressValidated(QuicConnection* connection) {
  connection->default_path_.validated = true;
}

// static
void QuicConnectionPeer::SendConnectionClosePacket(
    QuicConnection* connection,
    QuicIetfTransportErrorCodes ietf_error,
    QuicErrorCode error,
    const std::string& details) {
  connection->SendConnectionClosePacket(error, ietf_error, details);
}

// static
size_t QuicConnectionPeer::GetNumEncryptionLevels(QuicConnection* connection) {
  size_t count = 0;
  for (EncryptionLevel level :
       {ENCRYPTION_INITIAL, ENCRYPTION_HANDSHAKE, ENCRYPTION_ZERO_RTT,
        ENCRYPTION_FORWARD_SECURE}) {
    if (connection->framer_.HasEncrypterOfEncryptionLevel(level)) {
      ++count;
    }
  }
  return count;
}

// static
QuicNetworkBlackholeDetector& QuicConnectionPeer::GetBlackholeDetector(
    QuicConnection* connection) {
  return connection->blackhole_detector_;
}

// static
QuicAlarm* QuicConnectionPeer::GetBlackholeDetectorAlarm(
    QuicConnection* connection) {
  return connection->blackhole_detector_.alarm_.get();
}

// static
QuicTime QuicConnectionPeer::GetPathDegradingDeadline(
    QuicConnection* connection) {
  return connection->blackhole_detector_.path_degrading_deadline_;
}

// static
QuicTime QuicConnectionPeer::GetBlackholeDetectionDeadline(
    QuicConnection* connection) {
  return connection->blackhole_detector_.blackhole_deadline_;
}

// static
QuicTime QuicConnectionPeer::GetPathMtuReductionDetectionDeadline(
    QuicConnection* connection) {
  return connection->blackhole_detector_.path_mtu_reduction_deadline_;
}

// static
QuicTime QuicConnectionPeer::GetIdleNetworkDeadline(
    QuicConnection* connection) {
  return connection->idle_network_detector_.GetIdleNetworkDeadline();
}

// static
QuicAlarm* QuicConnectionPeer::GetIdleNetworkDetectorAlarm(
    QuicConnection* connection) {
  return connection->idle_network_detector_.alarm_.get();
}

// static
QuicIdleNetworkDetector& QuicConnectionPeer::GetIdleNetworkDetector(
    QuicConnection* connection) {
  return connection->idle_network_detector_;
}

// static
void QuicConnectionPeer::SetServerConnectionId(
    QuicConnection* connection,
    const QuicConnectionId& server_connection_id) {
  connection->default_path_.server_connection_id = server_connection_id;
  connection->InstallInitialCrypters(server_connection_id);
}

// static
size_t QuicConnectionPeer::NumUndecryptablePackets(QuicConnection* connection) {
  return connection->undecryptable_packets_.size();
}

void QuicConnectionPeer::SetConnectionClose(QuicConnection* connection) {
  connection->connected_ = false;
}

// static
void QuicConnectionPeer::SendPing(QuicConnection* connection) {
  connection->SendPingAtLevel(connection->encryption_level());
}

// static
void QuicConnectionPeer::SetLastPacketDestinationAddress(
    QuicConnection* connection,
    const QuicSocketAddress& address) {
  connection->last_received_packet_info_.destination_address = address;
}

// static
QuicPathValidator* QuicConnectionPeer::path_validator(
    QuicConnection* connection) {
  return &connection->path_validator_;
}

//  static
QuicByteCount QuicConnectionPeer::BytesSentOnAlternativePath(
    QuicConnection* connection) {
  return connection->alternative_path_.bytes_sent_before_address_validation;
}

//  static
QuicByteCount QuicConnectionPeer::BytesReceivedOnAlternativePath(
    QuicConnection* connection) {
  return connection->alternative_path_.bytes_received_before_address_validation;
}

// static
QuicConnectionId QuicConnectionPeer::GetClientConnectionIdOnAlternativePath(
    const QuicConnection* connection) {
  return connection->alternative_path_.client_connection_id;
}

// static
QuicConnectionId QuicConnectionPeer::GetServerConnectionIdOnAlternativePath(
    const QuicConnection* connection) {
  return connection->alternative_path_.server_connection_id;
}

// static
bool QuicConnectionPeer::IsAlternativePathValidated(
    QuicConnection* connection) {
  return connection->alternative_path_.validated;
}

// static
bool QuicConnectionPeer::IsAlternativePath(
    QuicConnection* connection,
    const QuicSocketAddress& self_address,
    const QuicSocketAddress& peer_address) {
  return connection->IsAlternativePath(self_address, peer_address);
}

// static
QuicByteCount QuicConnectionPeer::BytesReceivedBeforeAddressValidation(
    QuicConnection* connection) {
  return connection->default_path_.bytes_received_before_address_validation;
}

// static
void QuicConnectionPeer::ResetPeerIssuedConnectionIdManager(
    QuicConnection* connection) {
  connection->peer_issued_cid_manager_ = nullptr;
}

// static
QuicConnection::PathState* QuicConnectionPeer::GetDefaultPath(
    QuicConnection* connection) {
  return &connection->default_path_;
}

// static
QuicConnection::PathState* QuicConnectionPeer::GetAlternativePath(
    QuicConnection* connection) {
  return &connection->alternative_path_;
}

// static
void QuicConnectionPeer::RetirePeerIssuedConnectionIdsNoLongerOnPath(
    QuicConnection* connection) {
  connection->RetirePeerIssuedConnectionIdsNoLongerOnPath();
}

// static
bool QuicConnectionPeer::HasUnusedPeerIssuedConnectionId(
    const QuicConnection* connection) {
  return connection->peer_issued_cid_manager_->HasUnusedConnectionId();
}

// static
bool QuicConnectionPeer::HasSelfIssuedConnectionIdToConsume(
    const QuicConnection* connection) {
  return connection->self_issued_cid_manager_->HasConnectionIdToConsume();
}

// static
QuicSelfIssuedConnectionIdManager*
QuicConnectionPeer::GetSelfIssuedConnectionIdManager(
    QuicConnection* connection) {
  return connection->self_issued_cid_manager_.get();
}

// static
std::unique_ptr<QuicSelfIssuedConnectionIdManager>
QuicConnectionPeer::MakeSelfIssuedConnectionIdManager(
    QuicConnection* connection) {
  return connection->MakeSelfIssuedConnectionIdManager();
}

// static
void QuicConnectionPeer::SetLastDecryptedLevel(QuicConnection* connection,
                                               EncryptionLevel level) {
  connection->last_decrypted_packet_level_ = level;
}

}  // namespace test
}  // namespace quic
