-- Chaosnet over UDP dissector for Wireshark

chudp = Proto("chudp", "Chaosnet over UDP")

local CHUDP_HDR_LEN = 4
local CHAOS_HDR_LEN = 16
local HW_HDR_LEN = 6

local ef_too_short = ProtoExpert.new("chudp.too_short.expert", "Packet too short", expert.group.MALFORMED, expert.severity.ERROR)
chudp.experts = { ef_too_short }

function chudp.dissector(tvb, pinfo, tree)
   pinfo.cols.protocol = "CHUDP"

   local pktlen = tvb:reported_length_remaining()
   local pktlen_remaining = pktlen
   
   local subtree = tree:add(chudp, tvb:range(0, pktlen), "Chaosnet over UDP")

   if pktlen < CHUDP_HDR_LEN then
      tree:add_proto_expert_info(ef_too_short)
      return
   end

   -- Chaosnet UDP Header.
   local chudp_tree = subtree:add(tvb(0, 4), "CHUDP header")
   chudp_tree:add(tvb(0, 1), "Version: " .. tvb(0, 1))
   chudp_tree:add(tvb(1, 1), "Function: " .. tvb(1, 1))
   chudp_tree:add(tvb(2, 1), "Argument 1: " .. tvb(2, 1))
   chudp_tree:add(tvb(3, 1), "Argument 2: " .. tvb(3, 1))

   -- Chaosnet header.
   local chaos_tree = subtree:add(tvb(4, 16), "Chaosnet header")
   chaos_tree:add(tvb(4, 2), "Operation: " .. tvb(4, 2))
   chaos_tree:add(tvb(6, 2), "Count: " .. tvb(6, 2)) --- 4 bit forwardig count, 12 bit data count
   chaos_tree:add(tvb(8, 2), "Destination Address: " .. tvb(8, 2))
   chaos_tree:add(tvb(10, 2), "Destination Index: " .. tvb(10, 2))
   chaos_tree:add(tvb(12, 2), "Source Address: " .. tvb(12, 2))
   chaos_tree:add(tvb(14, 2), "Source Index: " .. tvb(14, 2))
   chaos_tree:add(tvb(16, 2), "Packet Number: " .. tvb(16, 2))
   chaos_tree:add(tvb(18, 2), "Acknowledge: " .. tvb(18, 2))

   -- Data.
   local d = tvb:range(6, 2)
   local foward_count = d:bitfield(0, 4)
   local data_count = d:bitfield(4, 12)
   local data_tree = subtree:add(tvb(20, data_count), "Data: " .. tvb(20, data_count))


   -- HW Trailer.
   local hw_offset = CHUDP_HDR_LEN + CHAOS_HDR_LEN + data_count
   local hw_tree = subtree:add(tvb(hw_offset, 6), "HW trailer")
   hw_tree:add(tvb(hw_offset + 0, 2), "Destination: " .. tvb(hw_offset + 0, 2))
   hw_tree:add(tvb(hw_offset + 2, 2), "Source: " .. tvb(hw_offset + 2, 2))
   hw_tree:add(tvb(hw_offset + 4, 2), "Checksum: " .. tvb(hw_offset + 4, 2))


   -- Fin.
   pktlen_remaining = pktlen_remaining - CHUDP_HDR_LEN - CHAOS_HDR_LEN - data_count
   return pktlen_remaining
end

udp_table = DissectorTable.get("udp.port")
udp_table:add(42042, chudp)
