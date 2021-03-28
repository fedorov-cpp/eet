#include "Device.h"

namespace Eet {

  Master::Master() :
    Device(Protocol::DeviceType::MASTER) {}


  void Master::pushActivate(const Protocol::Msg::Activate &msg) {
    (void) msg; // unused
  }


  void Master::pushHeartbeat(const Protocol::Msg::Heartbeat &msg) {
    switch(msg.m_commonFields.m_deviceType) {
      case Protocol::DeviceType::SLAVE: {
        if(Protocol::SlaveState::ACTIVE == msg.m_slaveState) {
          setCmdType(msg.m_cmdType);
          m_approveState = msg.m_approveState;
          ma_activeSlaves.set(static_cast<size_t>(msg.m_commonFields.m_deviceId - 1));
        } else {
          ma_activeSlaves.reset(static_cast<size_t>(msg.m_commonFields.m_deviceId - 1));
        }
        break;
      }
      case Protocol::DeviceType::MASTER: {
        break;
      }
      case Protocol::DeviceType::INVALID:
        break;
    }
  }


  void Master::pushCmd(const Protocol::Msg::Cmd &msg) {
    setCmdType(msg.m_cmdType);
    m_approveState = Protocol::ApproveState::NOT_APPROVED;
  }


  Protocol::Can::RawMsg Master::getActivateMsg() {
    // there should be no Cmd msg from Master
    return {};
  }


  void Master::setDeviceId(char id) {
    m_deviceId = id;
  }


  void Master::setNumOfMasters(size_t num) {
    m_numOfMasters = num;
  }


  void Master::setNumOfSlaves(size_t num) {
    m_numOfSlaves = num;
  }


  void Master::update() {
    using Protocol::SlaveState;

    m_errors = 0; // wipe errors
    m_errors |= (isDuplicatedDeviceId() << Protocol::DUPLICATED_DEVICE_ID);
    m_errors |= (isConWithSomeSlavesLost() << Protocol::CON_WITH_SOME_SLAVES_LOST);
    m_errors |= (isConWithAllSlavesLost() << Protocol::CON_WITH_ALL_SLAVES_LOST);
    m_errors |= (isConWithSomeMastersLost() << Protocol::CON_WITH_SOME_MASTERS_LOST);
    m_errors |= (isConWithAllMastersLost() << Protocol::CON_WITH_ALL_MASTERS_LOST);
    m_errors |= (isNoConnection() << Protocol::NO_CONNECTION);

    m_isAnyActiveSlave |= (0UL != ma_activeSlaves.count());
    m_errors |= (isNoActiveSlave() << Protocol::NO_ACTIVE_SLAVE);

    ma_activeSlaves.reset();
    ma_respondedMasters.reset();
    ma_respondedSlaves.reset();
    m_isAnyDuplicatedId = false;
    m_isAnyActiveSlave = false;
  }


  void Master::setCmdType(Protocol::CmdType cmdType) {
    m_cmdType = cmdType;
    m_approveState = Protocol::ApproveState::NOT_APPROVED;
  }

} // end namespace Eet