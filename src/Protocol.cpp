#include <cstring>
#include <string>

#include "Protocol.h"
#include "Helpers.h"

namespace Eet {

  Protocol::Msg::
  IMsg::IMsg(const CommonFields &commonFields, Can::Id canId) :
    m_commonFields(commonFields),
    m_canId(canId) {}


  Protocol::Msg::
  Activate::Activate(const CommonFields &commonFields) :
    IMsg(commonFields, Can::Id::ACTIVATE) {}


  uint16_t Protocol::Msg::Activate::isNotValid() {
    return m_commonFields.isNotValid();
  }


  Protocol::Msg::Activate::operator Can::RawMsg() {
    Can::RawMsg msg{};
    msg.m_canId = static_cast<uint32_t>(m_canId);
    msg.m_dlc = DLC;
    msg.m_dataL = 0UL;
    msg.m_dataH = 0UL;

    Helpers::setBits(msg.m_dataL, m_commonFields.m_deviceId, 0, 0, 6);
    Helpers::setBits(msg.m_dataL, m_commonFields.m_deviceType, 0, 6, 2);
    Helpers::setBits(msg.m_dataL, m_commonFields.m_errors, 1, 0, 8);
    return msg;
  }


  Protocol::Msg::
  Heartbeat::Heartbeat(const CommonFields &commonFields, SlaveState slaveState,
                       ApproveState approveState, CmdType cmdType) :
    IMsg(commonFields, Can::Id::HEARTBEAT),
    m_slaveState(slaveState),
    m_approveState(approveState),
    m_cmdType(cmdType) {}


  uint16_t Protocol::Msg::Heartbeat::isNotValid() {
    uint16_t ret = m_commonFields.isNotValid();
    ret |= ((Protocol::SlaveState::INVALID == m_slaveState) << INVALID_SLAVE_STATE);
    ret |= ((Protocol::ApproveState::INVALID == m_approveState) << INVALID_APPROVE_STATE);
    ret |= ((Protocol::CmdType::INVALID == m_cmdType) << INVALID_CMD_TYPE);
    return ret;
  }


  Protocol::Msg::Heartbeat::operator Can::RawMsg() {
    Can::RawMsg msg{};
    msg.m_canId = static_cast<uint32_t>(m_canId);
    msg.m_dlc = DLC;
    msg.m_dataL = 0UL;
    msg.m_dataH = 0UL;

    Helpers::setBits(msg.m_dataL, m_commonFields.m_deviceId, 0, 0, 6);
    Helpers::setBits(msg.m_dataL, m_commonFields.m_deviceType, 0, 6, 2);
    Helpers::setBits(msg.m_dataL, m_commonFields.m_errors, 1, 0, 8);
    Helpers::setBits(msg.m_dataL, m_slaveState, 2, 0, 1);
    Helpers::setBits(msg.m_dataL, m_approveState, 2, 1, 1);
    Helpers::setBits(msg.m_dataL, m_cmdType, 2, 2, 6);
    return msg;
  }


  Protocol::Msg::
  Cmd::Cmd(const CommonFields &commonFields, CmdType cmdType) :
    IMsg(commonFields, Can::Id::CMD),
    m_cmdType(cmdType) {}


  Protocol::Msg::Cmd::operator Can::RawMsg() {
    Can::RawMsg msg{};
    msg.m_canId = static_cast<uint32_t>(m_canId);
    msg.m_dlc = DLC;
    msg.m_dataL = 0UL;
    msg.m_dataH = 0UL;

    Helpers::setBits(msg.m_dataL, m_commonFields.m_deviceId, 0, 0, 6);
    Helpers::setBits(msg.m_dataL, m_commonFields.m_deviceType, 0, 6, 2);
    Helpers::setBits(msg.m_dataL, m_commonFields.m_errors, 1, 0, 8);
    Helpers::setBits(msg.m_dataL, m_cmdType, 2, 2, 6);
    return msg;
  }


  uint16_t Protocol::Msg::Cmd::isNotValid() {
    uint16_t ret = m_commonFields.isNotValid();
    ret |= ((Protocol::CmdType::INVALID == m_cmdType) << INVALID_CMD_TYPE);
    return ret;
  }


  bool Protocol::DeviceId::isCorrectId(char id) {
    return ((id >= MIN_DEVICE_ID) && (id <= MAX_DEVICE_ID));
  }


  Protocol::Msg::
  CommonFields::CommonFields(char id, DeviceType type, uint8_t e) :
    m_deviceId(id),
    m_deviceType(type),
    m_errors(e) {}


  Protocol::Msg::
  CommonFields::CommonFields(uint32_t data) :
    CommonFields(
      Helpers::bits2type<char>(data, 0U, 0U, 6U),
      Helpers::enum2type(data, 0U, 6U, 2U,
                         DeviceType::MASTER,
                         DeviceType::SLAVE),
      Helpers::bits2type<char>(data, 1U, 0U, 8U)
    ) {}


  uint16_t Protocol::Msg::CommonFields::isNotValid() {
    uint16_t ret = 0U;
    ret |= (not Protocol::DeviceId::isCorrectId(m_deviceId)
      << Protocol::Msg::Errors::INVALID_DEVICE_ID);
    ret |= ((Protocol::DeviceType::INVALID == m_deviceType)
      << Protocol::Msg::Errors::INVALID_DEVICE_TYPE);
    return ret;
  }


  Protocol::Msg::
  LogMsg::LogMsg(char tDeviceId, uint16_t errors, const Can::RawMsg &msg) {
    clean();
    size_t pos = 0UL;

    // $
    m_msg[pos++] = '$';
    // <LogMsgType>,
    m_msg[pos++] = 'O'; // got msg from (O)ther device
    m_msg[pos++] = ',';
    // <tDeviceId>,
    if(Protocol::DeviceId::INVALID == tDeviceId) {
      std::memcpy(m_msg+pos, "INVALID_DEVICE_ID", 17);
      pos += 17;
    } else {
      auto tDeviceIdStr = std::to_string(static_cast<unsigned>(tDeviceId));
      std::memcpy(m_msg + pos, tDeviceIdStr.c_str(), tDeviceIdStr.size());
      pos += tDeviceIdStr.size();
    }
    m_msg[pos++] = ',';
    // <CanId>,
    auto canId = static_cast<Protocol::Can::Id>(msg.m_canId);
    switch(canId) {
      case Protocol::Can::Id::ACTIVATE:
        m_msg[pos++] = 'A';
        break;
      case Protocol::Can::Id::HEARTBEAT:
        m_msg[pos++] = 'H';
        break;
      case Protocol::Can::Id::CMD:
        m_msg[pos++] = 'C';
        break;
      default:
        std::memcpy(m_msg+pos, "INVALID_CAN_ID", 14);
        pos += 14;
        break;
    }
    m_msg[pos++] = ',';
    // <CanDlc>,
    if((errors >> Protocol::Msg::Errors::INVALID_CAN_DLC) & 0x01) {
      constexpr char invalidCanDlcStr[] = "INVALID_CAN_DLC";
      std::memcpy(m_msg+pos, invalidCanDlcStr, 15);
      pos += 15;
    } else {
      auto canDlcStr = std::to_string(msg.m_dlc);
      std::memcpy(m_msg+pos, canDlcStr.c_str(), canDlcStr.size());
      pos += canDlcStr.size();
    }
    m_msg[pos++] = ',';
    // <oDeviceId>,
    Protocol::Msg::CommonFields commonFields(msg.m_dataL);
    if(Protocol::DeviceId::INVALID == commonFields.m_deviceId) {
      std::memcpy(m_msg+pos, "O_INVALID_DEVICE_ID", 19);
      pos += 19;
    } else {
      auto oDeviceIdStr = std::to_string(static_cast<unsigned>(commonFields.m_deviceId));
      std::memcpy(m_msg + pos, oDeviceIdStr.c_str(), oDeviceIdStr.size());
      pos += oDeviceIdStr.size();
    }
    m_msg[pos++] = ',';
    // <oDuplicatedId>,
    if((errors >> Protocol::Msg::Errors::DUPLICATED_DEVICE_ID) & 0x01) {
      constexpr char oDuplicatedIdStr[] = "O_DUPLICATED_ID";
      std::memcpy(m_msg+pos, oDuplicatedIdStr, 15);
      pos += 15;
    }
    m_msg[pos++] = ',';
    // <DeviceType>,
    switch(commonFields.m_deviceType) {
      case Protocol::DeviceType::MASTER: {
        std::memcpy(m_msg+pos, "MASTER", 6);
        pos += 6;
        break;
      }
      case Protocol::DeviceType::SLAVE: {
        std::memcpy(m_msg+pos, "SLAVE", 5);
        pos += 5;
        break;
      }
      default: {
        std::memcpy(m_msg+pos, "INVALID_DEVICE_TYPE", 19);
        pos += 19;
        break;
      }
    }
    m_msg[pos++] = ',';
    // <eDuplicatedDeviceId>,
    if((commonFields.m_errors >> Protocol::ErrorBit::DUPLICATED_DEVICE_ID) & 0x01) {
      std::memcpy(m_msg+pos, "E_DUPLICATED_DEVICE_ID", 22);
      pos += 22;
    }
    m_msg[pos++] = ',';
    // <eConWithSomeSlavesLost>,
    if((commonFields.m_errors >> Protocol::ErrorBit::CON_WITH_SOME_SLAVES_LOST) & 0x01) {
      std::memcpy(m_msg+pos, "E_SOME_SLAVES_LOST", 18);
      pos += 18;
    }
    m_msg[pos++] = ',';
    // <eConWithAllSlavesLost>,
    if((commonFields.m_errors >> Protocol::ErrorBit::CON_WITH_ALL_SLAVES_LOST) & 0x01) {
      std::memcpy(m_msg+pos, "E_ALL_SLAVES_LOST", 17);
      pos += 17;
    }
    m_msg[pos++] = ',';
    // <eConWithSomeMastersLost>,
    if((commonFields.m_errors >> Protocol::ErrorBit::CON_WITH_SOME_MASTERS_LOST) & 0x01) {
      std::memcpy(m_msg+pos, "E_SOME_MASTERS_LOST", 19);
      pos += 19;
    }
    m_msg[pos++] = ',';
    // <eConWithAllMastersLost>,
    if((commonFields.m_errors >> Protocol::ErrorBit::CON_WITH_ALL_MASTERS_LOST) & 0x01) {
      std::memcpy(m_msg+pos, "E_ALL_MASTERS_LOST", 18);
      pos += 18;
    }
    m_msg[pos++] = ',';
    // <eNoConnection>,
    if((commonFields.m_errors >> Protocol::ErrorBit::NO_CONNECTION) & 0x01) {
      std::memcpy(m_msg+pos, "E_NO_CONNECTION", 15);
      pos += 15;
    }
    m_msg[pos++] = ',';
    // <eNoActiveSlave>,
    if((commonFields.m_errors >> Protocol::ErrorBit::NO_ACTIVE_SLAVE) & 0x01) {
      std::memcpy(m_msg+pos, "E_NO_ACTIVE_SLAVE", 17);
      pos += 17;
    }
    m_msg[pos++] = ',';
    // <SlaveState>,
    if((Protocol::Can::Id::ACTIVATE != canId) &&
       (Protocol::Can::Id::CMD != canId) &&
       (Protocol::DeviceType::MASTER != commonFields.m_deviceType)) {
      auto slaveState = Helpers::enum2type(msg.m_dataL, 2U, 0U, 1U,
                                           Protocol::SlaveState::NOT_ACTIVE,
                                           Protocol::SlaveState::ACTIVE);
      switch(slaveState) {
        case Protocol::SlaveState::NOT_ACTIVE: {
          std::memcpy(m_msg+pos, "NOT_ACTIVE", 10);
          pos += 10;
          break;
        }
        case Protocol::SlaveState::ACTIVE: {
          std::memcpy(m_msg+pos, "ACTIVE", 6);
          pos += 6;
          break;
        }
        default: {
          std::memcpy(m_msg+pos, "INVALID_SLAVE_STATE", 19);
          pos += 19;
          break;
        }
      }
    }
    m_msg[pos++] = ',';
    // <ApproveState>,
    if((Protocol::Can::Id::ACTIVATE != canId) &&
       (Protocol::Can::Id::CMD != canId)) {
      auto approveState = Helpers::enum2type(msg.m_dataL, 2U, 1U, 1U,
                                             Protocol::ApproveState::NOT_APPROVED,
                                             Protocol::ApproveState::APPROVED);
      switch(approveState) {
        case Protocol::ApproveState::APPROVED: {
          std::memcpy(m_msg+pos, "APPROVED", 8);
          pos += 8;
          break;
        }
        case Protocol::ApproveState::NOT_APPROVED: {
          std::memcpy(m_msg+pos, "NOT_APPROVED", 12);
          pos += 12;
          break;
        }
        default: {
          std::memcpy(m_msg+pos, "INVALID_APPROVE_STATE", 21);
          pos += 21;
          break;
        }
      }
    }
    m_msg[pos++] = ',';
    // <CmdType>,
    if(Protocol::Can::Id::ACTIVATE != canId) {
      auto cmdType = Helpers::enum2type(msg.m_dataL, 2U, 2U, 7U,
                                        Protocol::CmdType::COMPLETE,
                                        Protocol::CmdType::FULL_ASTERN);
      switch(cmdType) {
        case Protocol::CmdType::COMPLETE: {
          std::memcpy(m_msg+pos, "COMPLETE", 8);
          pos += 8;
          break;
        }
        case Protocol::CmdType::GET_READY: {
          std::memcpy(m_msg+pos, "GET_READY", 9);
          pos += 9;
          break;
        }
        case Protocol::CmdType::FULL_AHEAD: {
          std::memcpy(m_msg+pos, "FULL_AHEAD", 10);
          pos += 10;
          break;
        }
        case Protocol::CmdType::HALF_AHEAD: {
          std::memcpy(m_msg+pos, "HALF_AHEAD", 10);
          pos += 10;
          break;
        }
        case Protocol::CmdType::SLOW_AHEAD: {
          std::memcpy(m_msg+pos, "SLOW_AHEAD", 10);
          pos += 10;
          break;
        }
        case Protocol::CmdType::DEAD_SLOW_AHEAD: {
          std::memcpy(m_msg+pos, "DEAD_SLOW_AHEAD", 15);
          pos += 15;
          break;
        }
        case Protocol::CmdType::STOP: {
          std::memcpy(m_msg+pos, "STOP", 4);
          pos += 4;
          break;
        }
        case Protocol::CmdType::DEAD_SLOW_ASTERN: {
          std::memcpy(m_msg+pos, "DEAD_SLOW_ASTERN", 16);
          pos += 16;
          break;
        }
        case Protocol::CmdType::SLOW_ASTERN: {
          std::memcpy(m_msg+pos, "SLOW_ASTERN", 11);
          pos += 11;
          break;
        }
        case Protocol::CmdType::HALF_ASTERN: {
          std::memcpy(m_msg+pos, "HALF_ASTERN", 11);
          pos += 11;
          break;
        }
        case Protocol::CmdType::FULL_ASTERN: {
          std::memcpy(m_msg+pos, "FULL_ASTERN", 11);
          pos += 11;
          break;
        }
        default: {
          std::memcpy(m_msg+pos, "INVALID_CMD_TYPE", 16);
          pos += 16;
          break;
        }
      }
    }
    m_msg[pos++] = ',';
    // \r
    m_msg[pos++] = '\r';
    // \n
    m_msg[pos++] = '\n';

    m_msgSize = pos;
  }


  Protocol::Msg::
  LogMsg::LogMsg(char deviceId, DeviceType deviceType, char errors,
                 SlaveState slaveState, ApproveState approveState, CmdType cmdType) {
    clean();
    size_t pos = 0UL;

    // $
    m_msg[pos++] = '$';
    // <LogMsgType>,
    m_msg[pos++] = 'T'; // got msg from (T)his device
    m_msg[pos++] = ',';
    // <tDeviceId>,
    if(Protocol::DeviceId::INVALID == deviceId) {
      std::memcpy(m_msg+pos, "INVALID_DEVICE_ID", 17);
      pos += 17;
    } else {
      auto tDeviceIdStr = std::to_string(static_cast<unsigned>(deviceId));
      std::memcpy(m_msg + pos, tDeviceIdStr.c_str(), tDeviceIdStr.size());
      pos += tDeviceIdStr.size();
    }
    m_msg[pos++] = ',';
    // <DeviceType>,
    switch(deviceType) {
      case Protocol::DeviceType::MASTER: {
        std::memcpy(m_msg+pos, "MASTER", 6);
        pos += 6;
        break;
      }
      case Protocol::DeviceType::SLAVE: {
        std::memcpy(m_msg+pos, "SLAVE", 5);
        pos += 5;
        break;
      }
      default: {
        std::memcpy(m_msg+pos, "INVALID_DEVICE_TYPE", 19);
        pos += 19;
        break;
      }
    }
    m_msg[pos++] = ',';
    // <eDuplicatedDeviceId>,
    if((errors >> Protocol::ErrorBit::DUPLICATED_DEVICE_ID) & 0x01) {
      std::memcpy(m_msg+pos, "E_DUPLICATED_DEVICE_ID", 22);
      pos += 22;
    }
    m_msg[pos++] = ',';
    // <eConWithSomeSlavesLost>,
    if((errors >> Protocol::ErrorBit::CON_WITH_SOME_SLAVES_LOST) & 0x01) {
      std::memcpy(m_msg+pos, "E_SOME_SLAVES_LOST", 18);
      pos += 18;
    }
    m_msg[pos++] = ',';
    // <eConWithAllSlavesLost>,
    if((errors >> Protocol::ErrorBit::CON_WITH_ALL_SLAVES_LOST) & 0x01) {
      std::memcpy(m_msg+pos, "E_ALL_SLAVES_LOST", 17);
      pos += 17;
    }
    m_msg[pos++] = ',';
    // <eConWithSomeMastersLost>,
    if((errors >> Protocol::ErrorBit::CON_WITH_SOME_MASTERS_LOST) & 0x01) {
      std::memcpy(m_msg+pos, "E_SOME_MASTERS_LOST", 19);
      pos += 19;
    }
    m_msg[pos++] = ',';
    // <eConWithAllMastersLost>,
    if((errors >> Protocol::ErrorBit::CON_WITH_ALL_MASTERS_LOST) & 0x01) {
      std::memcpy(m_msg+pos, "E_ALL_MASTERS_LOST", 18);
      pos += 18;
    }
    m_msg[pos++] = ',';
    // <eNoConnection>,
    if((errors >> Protocol::ErrorBit::NO_CONNECTION) & 0x01) {
      std::memcpy(m_msg+pos, "E_NO_CONNECTION", 15);
      pos += 15;
    }
    m_msg[pos++] = ',';
    // <eNoActiveSlave>,
    if((errors >> Protocol::ErrorBit::NO_ACTIVE_SLAVE) & 0x01) {
      std::memcpy(m_msg+pos, "E_NO_ACTIVE_SLAVE", 17);
      pos += 17;
    }
    m_msg[pos++] = ',';
    // <SlaveState>,
    if(Protocol::DeviceType::MASTER != deviceType) {
      switch(slaveState) {
        case Protocol::SlaveState::NOT_ACTIVE: {
          std::memcpy(m_msg+pos, "NOT_ACTIVE", 10);
          pos += 10;
          break;
        }
        case Protocol::SlaveState::ACTIVE: {
          std::memcpy(m_msg+pos, "ACTIVE", 6);
          pos += 6;
          break;
        }
        default: {
          std::memcpy(m_msg+pos, "INVALID_SLAVE_STATE", 19);
          pos += 19;
          break;
        }
      }
    }
    m_msg[pos++] = ',';
    // <ApproveState>,
    switch(approveState) {
      case Protocol::ApproveState::APPROVED: {
        std::memcpy(m_msg+pos, "APPROVED", 8);
        pos += 8;
        break;
      }
      case Protocol::ApproveState::NOT_APPROVED: {
        std::memcpy(m_msg+pos, "NOT_APPROVED", 12);
        pos += 12;
        break;
      }
      default: {
        std::memcpy(m_msg+pos, "INVALID_APPROVE_STATE", 21);
        pos += 21;
        break;
      }
    }
    m_msg[pos++] = ',';
    // <CmdType>,
    switch(cmdType) {
      case Protocol::CmdType::COMPLETE: {
        std::memcpy(m_msg+pos, "COMPLETE", 8);
        pos += 8;
        break;
      }
      case Protocol::CmdType::GET_READY: {
        std::memcpy(m_msg+pos, "GET_READY", 9);
        pos += 9;
        break;
      }
      case Protocol::CmdType::FULL_AHEAD: {
        std::memcpy(m_msg+pos, "FULL_AHEAD", 10);
        pos += 10;
        break;
      }
      case Protocol::CmdType::HALF_AHEAD: {
        std::memcpy(m_msg+pos, "HALF_AHEAD", 10);
        pos += 10;
        break;
      }
      case Protocol::CmdType::SLOW_AHEAD: {
        std::memcpy(m_msg+pos, "SLOW_AHEAD", 10);
        pos += 10;
        break;
      }
      case Protocol::CmdType::DEAD_SLOW_AHEAD: {
        std::memcpy(m_msg+pos, "SLOWEST_AHEAD", 13);
        pos += 13;
        break;
      }
      case Protocol::CmdType::STOP: {
        std::memcpy(m_msg+pos, "STOP", 4);
        pos += 4;
        break;
      }
      case Protocol::CmdType::DEAD_SLOW_ASTERN: {
        std::memcpy(m_msg+pos, "DEAD_SLOW_ASTERN", 16);
        pos += 16;
        break;
      }
      case Protocol::CmdType::SLOW_ASTERN: {
        std::memcpy(m_msg+pos, "SLOW_ASTERN", 11);
        pos += 11;
        break;
      }
      case Protocol::CmdType::HALF_ASTERN: {
        std::memcpy(m_msg+pos, "HALF_ASTERN", 11);
        pos += 11;
        break;
      }
      case Protocol::CmdType::FULL_ASTERN: {
        std::memcpy(m_msg+pos, "FULL_ASTERN", 11);
        pos += 11;
        break;
      }
      default: {
        std::memcpy(m_msg+pos, "INVALID_CMD_TYPE", 16);
        pos += 16;
        break;
      }
    }
    m_msg[pos++] = ',';
    // \r
    m_msg[pos++] = '\r';
    // \n
    m_msg[pos++] = '\n';

    m_msgSize = pos;
  }


  void Protocol::Msg::
  LogMsg::clean() {
    std::memset(m_msg, 0, m_msgSize);
    m_msgSize = 0UL;
  }
} // end namespace Eet