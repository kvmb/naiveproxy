// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QUICHE_QUIC_CORE_HTTP_SPDY_UTILS_H_
#define QUICHE_QUIC_CORE_HTTP_SPDY_UTILS_H_

#include <cstddef>
#include <cstdint>
#include <string>

#include "absl/types/optional.h"
#include "quic/core/http/http_constants.h"
#include "quic/core/http/quic_header_list.h"
#include "quic/core/quic_packets.h"
#include "quic/platform/api/quic_export.h"
#include "spdy/core/spdy_alt_svc_wire_format.h"
#include "spdy/core/spdy_header_block.h"

namespace quic {

class QUIC_EXPORT_PRIVATE SpdyUtils {
 public:
  SpdyUtils() = delete;

  // Populate |content length| with the value of the content-length header.
  // Returns true on success, false if parsing fails or content-length header is
  // missing.
  static bool ExtractContentLengthFromHeaders(int64_t* content_length,
                                              spdy::SpdyHeaderBlock* headers);

  // Copies a list of headers to a SpdyHeaderBlock.
  static bool CopyAndValidateHeaders(const QuicHeaderList& header_list,
                                     int64_t* content_length,
                                     spdy::SpdyHeaderBlock* headers);

  // Copies a list of headers to a SpdyHeaderBlock.
  // If |expect_final_byte_offset| is true, requires exactly one header field
  // with key kFinalOffsetHeaderKey and an integer value.
  // If |expect_final_byte_offset| is false, no kFinalOffsetHeaderKey may be
  // present.
  // Returns true if parsing is successful.  Returns false if the presence of
  // kFinalOffsetHeaderKey does not match the value of
  // |expect_final_byte_offset|, the kFinalOffsetHeaderKey value cannot be
  // parsed, any other pseudo-header is present, an empty header key is present,
  // or a header key contains an uppercase character.
  static bool CopyAndValidateTrailers(const QuicHeaderList& header_list,
                                      bool expect_final_byte_offset,
                                      size_t* final_byte_offset,
                                      spdy::SpdyHeaderBlock* trailers);

  // Populates the fields of |headers| to make a GET request of |url|,
  // which must be fully-qualified.
  static bool PopulateHeaderBlockFromUrl(const std::string url,
                                         spdy::SpdyHeaderBlock* headers);

  // Parses the "datagram-flow-id" header, returns the flow ID on success, or
  // returns absl::nullopt if the header was not present or failed to parse.
  static absl::optional<QuicDatagramStreamId> ParseDatagramFlowIdHeader(
      const spdy::SpdyHeaderBlock& headers);

  // Adds the "datagram-flow-id" header.
  static void AddDatagramFlowIdHeader(spdy::SpdyHeaderBlock* headers,
                                      QuicDatagramStreamId flow_id);

  // Returns the advertised QUIC version from the specified alternative service
  // advertisement, or ParsedQuicVersion::Unsupported() if no supported version
  // is advertised.
  static ParsedQuicVersion ExtractQuicVersionFromAltSvcEntry(
      const spdy::SpdyAltSvcWireFormat::AlternativeService&
          alternative_service_entry,
      const ParsedQuicVersionVector& supported_versions);
};

}  // namespace quic

#endif  // QUICHE_QUIC_CORE_HTTP_SPDY_UTILS_H_
