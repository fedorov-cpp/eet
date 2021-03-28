#include "Device.h"
#include "Helpers.h"

namespace Eet {

  Device::Device(Protocol::DeviceType deviceType) :
    m_deviceType(deviceType),
    m_deviceId(Protocol::DeviceId::INVALID),
    m_errors(0),
    m_numOfMasters(0),
    m_numOfSlaves(0),
    m_isAnyDuplicatedId(false),
    m_isAnyActiveSlave(false),
    m_slaveState(Protocol::SlaveState::NOT_ACTIVE),
    m_cmdType(Protocol::CmdType::INVALID),
    m_approveState(Protocol::ApproveState::NOT_APPROVED) {}


  uint16_t Device::pushMsg(const Protocol::Can::RawMsg &rawMsg) {
    uint16_t notValid = 0U;

    Protocol::Msg::CommonFields commonFileds(rawMsg.m_dataL);
    notValid |= commonFileds.isNotValid();
    if(not notValid) {
      notValid |= registerResponderId(commonFileds.m_deviceId,
                                      commonFileds.m_deviceType);
      if(not notValid) {
        switch(static_cast<Protocol::Can::Id>(rawMsg.m_canId)) {
          case Protocol::Can::Id::ACTIVATE: {
            if(Protocol::Msg::Activate::DLC != rawMsg.m_dlc) {
              notValid |= (1U << Protocol::Msg::Errors::INVALID_CAN_DLC);
            } else {
              Protocol::Msg::Activate activateMsg(commonFileds);
              notValid |= activateMsg.isNotValid();
              if(not notValid) {
                pushActivate(activateMsg);
              }
            }
            break;
          }
          case Protocol::Can::Id::HEARTBEAT: {
            if(Protocol::Msg::Heartbeat::DLC != rawMsg.m_dlc) {
              notValid = (1U << Protocol::Msg::Errors::INVALID_CAN_DLC);
            } else {
              auto slaveState = Helpers::enum2type(rawMsg.m_dataL, 2U, 0U, 1U,
                                                   Protocol::SlaveState::NOT_ACTIVE,
                                                   Protocol::SlaveState::ACTIVE);
              auto approveState = Helpers::enum2type(rawMsg.m_dataL, 2U, 1U, 1U,
                                                     Protocol::ApproveState::NOT_APPROVED,
                                                     Protocol::ApproveState::APPROVED);
              auto cmdType = Helpers::enum2type(rawMsg.m_dataL, 2U, 2U, 7U,
                                                Protocol::CmdType::COMPLETE,
                                                Protocol::CmdType::FULL_ASTERN);
              Protocol::Msg::Heartbeat heartbeatMsg(commonFileds, slaveState,
                                                    approveState, cmdType);
              notValid |= heartbeatMsg.isNotValid();
              if(not notValid) {
                pushHeartbeat(heartbeatMsg);
              }
            }
            break;
          }
          case Protocol::Can::Id::CMD: {
            if(Protocol::Msg::Cmd::DLC != rawMsg.m_dlc) {
              notValid |= (1U << Protocol::Msg::Errors::INVALID_CAN_DLC);
            } else {
              auto cmdType = Helpers::enum2type(rawMsg.m_dataL, 2U, 2U, 7U,
                                                Protocol::CmdType::COMPLETE,
                                                Protocol::CmdType::FULL_ASTERN);
              Protocol::Msg::Cmd cmdMsg(commonFileds, cmdType);
              notValid |= cmdMsg.isNotValid();
              if(not notValid) {
                pushCmd(cmdMsg);
              }
            }
            break;
          }
          default:
            notValid |= (1U << Protocol::Msg::Errors::INVALID_CAN_ID);
            break;
        }
      }
    }

    return notValid;
  }


  char Device::getDeviceId() const {
    return m_deviceId;
  }


  char Device::getErrors() const {
    return m_errors;
  }


  Protocol::DeviceType Device::getDeviceType() {
    return m_deviceType;
  }


  Protocol::SlaveState Device::getSlaveState() {
    return m_slaveState;
  }


  Protocol::ApproveState Device::getApproveState() {
    return m_approveState;
  }


  Protocol::CmdType Device::getCmdType() {
    return m_cmdType;
  }


  Protocol::Can::RawMsg Device::getActivateMsg() {
    Protocol::Msg::CommonFields commonFields(m_deviceId, m_deviceType, m_errors);
    Protocol::Msg::Activate activateMsg(commonFields);
    return static_cast<Protocol::Can::RawMsg>(activateMsg);
  }


  Protocol::Can::RawMsg Device::getHeartbeatMsg() {
    Protocol::Msg::CommonFields commonFields(m_deviceId, m_deviceType, m_errors);
    Protocol::Msg::Heartbeat heartbeatMsg(commonFields, m_slaveState, m_approveState, m_cmdType);
    return static_cast<Protocol::Can::RawMsg>(heartbeatMsg);
  }


  Protocol::Can::RawMsg Device::getCmdMsg() {
    Protocol::Msg::CommonFields commonFields(m_deviceId, m_deviceType, m_errors);
    Protocol::Msg::Cmd cmdMsg(commonFields, m_cmdType);
    return static_cast<Protocol::Can::RawMsg>(cmdMsg);
  }


  bool Device::isAnyConLost() {
    bool ret = false;
    ret |= ((m_errors >> Protocol::CON_WITH_SOME_SLAVES_LOST) & 0x01);
    ret |= ((m_errors >> Protocol::CON_WITH_SOME_MASTERS_LOST) & 0x01);
    return ret;
  }


  bool Device::isAnyCon() {
    bool ret = not ((m_errors >> Protocol::NO_CONNECTION) & 0x01);
    return ret;
  }


  bool Device::isAnyActiveSlave() {
      return not ((m_errors >> Protocol::NO_ACTIVE_SLAVE) & 0x01);
  }


  bool Device::isDuplicatedDeviceId() {
    return m_isAnyDuplicatedId;
  }


  bool Device::isAnyError() {
      return static_cast<bool>(m_errors);
  }


  bool Device::isConWithSomeSlavesLost() {
    auto total = ma_respondedSlaves.count();
    return total < m_numOfSlaves;
  }


  bool Device::isConWithAllSlavesLost() {
    auto total = ma_respondedSlaves.count();
    return (0UL == total) && (0UL != m_numOfSlaves);
  }


  bool Device::isConWithSomeMastersLost() {
    auto total = ma_respondedMasters.count();
    return total < m_numOfMasters;
  }


  bool Device::isConWithAllMastersLost() {
    auto total = ma_respondedMasters.count();
    return (0UL == total) && (0UL != m_numOfMasters);
  }


  bool Device::isNoConnection() {
    return (isConWithAllSlavesLost() || (0UL == m_numOfSlaves)) && (isConWithAllMastersLost() || (0UL == m_numOfMasters));
  }


  bool Device::isNoActiveSlave() {
    return ((not m_isAnyActiveSlave) &&
            (Protocol::SlaveState::ACTIVE != m_slaveState));
  }


  uint16_t
  Device::registerResponderId(char deviceId, Protocol::DeviceType deviceType) {
    uint16_t ret = 0U;
    if(deviceId != m_deviceId) { // If message from another node
      switch(deviceType) {
        case Protocol::DeviceType::SLAVE:
          ma_respondedSlaves.set(static_cast<size_t>(deviceId - 1));
          break;
        case Protocol::DeviceType::MASTER:
          ma_respondedMasters.set(static_cast<size_t>(deviceId - 1));
          break;
        default:
          ret = (1U << Protocol::Msg::Errors::INVALID_DEVICE_TYPE);
      }
    } else {
      m_isAnyDuplicatedId = true;
      ret = (1U << Protocol::Msg::Errors::DUPLICATED_DEVICE_ID);
    }
    return ret;
  }

} // end namespace Eet