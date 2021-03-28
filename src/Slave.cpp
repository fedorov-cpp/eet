#include "Device.h"
#include "Helpers.h"

namespace Eet {

  Slave::Slave() :
    Device(Protocol::DeviceType::SLAVE),
    m_isActivating(false) {}


  void Slave::setCmdType(Eet::Protocol::CmdType cmdType) {
    m_cmdType = cmdType;
  }


  Protocol::Can::RawMsg Slave::getCmdMsg() {
    // there should be no Cmd msg from Slave
    return {};
  }


  void Slave::setDeviceId(char id) {
    m_deviceId = id;
  }


  void Slave::setNumOfMasters(size_t num) {
    m_numOfMasters = num;
  }


  void Slave::setNumOfSlaves(size_t num) {
    m_numOfSlaves = num;
  }


  void Slave::pushActivate(const Protocol::Msg::Activate &msg) {
    if(Protocol::DeviceType::SLAVE == msg.m_commonFields.m_deviceType) {
      m_slaveState = Protocol::SlaveState::NOT_ACTIVE;
    }
  }


  void Slave::pushHeartbeat(const Protocol::Msg::Heartbeat &msg) {
    switch(msg.m_slaveState) {
      case Protocol::SlaveState::ACTIVE:
        setCmdType(msg.m_cmdType);
        m_approveState = msg.m_approveState;
        ma_activeSlaves.set(static_cast<size_t>(msg.m_commonFields.m_deviceId-1));
        break;
      case Protocol::SlaveState::NOT_ACTIVE:
        ma_activeSlaves.reset(static_cast<size_t>(msg.m_commonFields.m_deviceId-1));
        break;
      case Protocol::SlaveState::INVALID:
        break;
    }
  }


  void Slave::pushCmd(const Protocol::Msg::Cmd &msg) {
    if(Protocol::DeviceType::MASTER == msg.m_commonFields.m_deviceType) {
      setCmdType(msg.m_cmdType);
      m_approveState = Protocol::ApproveState::NOT_APPROVED;
    }
  }


  void Slave::update() {
    using Protocol::SlaveState;

    m_errors = 0; // wipe errors
    m_errors |= (isDuplicatedDeviceId() << Protocol::DUPLICATED_DEVICE_ID);
    m_errors |= (isConWithSomeSlavesLost() << Protocol::CON_WITH_SOME_SLAVES_LOST);
    m_errors |= (isConWithAllSlavesLost() << Protocol::CON_WITH_ALL_SLAVES_LOST);
    m_errors |= (isConWithSomeMastersLost() << Protocol::CON_WITH_SOME_MASTERS_LOST);
    m_errors |= (isConWithAllMastersLost() << Protocol::CON_WITH_ALL_MASTERS_LOST);
    m_errors |= (isNoConnection() << Protocol::NO_CONNECTION);

    m_isAnyActiveSlave |= (0UL != ma_activeSlaves.count());
    if(m_isAnyActiveSlave || isNoConnection()) {
      m_slaveState = SlaveState::NOT_ACTIVE;
    } else if(m_isActivating) {
      m_slaveState = SlaveState::ACTIVE;
    }
    m_isActivating = false;
    m_errors |= (isNoActiveSlave() << Protocol::NO_ACTIVE_SLAVE);

    ma_activeSlaves.reset();
    ma_respondedMasters.reset();
    ma_respondedSlaves.reset();
    m_isAnyDuplicatedId = false;
    m_isAnyActiveSlave = false;
  }


  void Slave::activate() {
    m_isActivating = ((Protocol::SlaveState::NOT_ACTIVE == m_slaveState) && isAnyCon());
  }

  bool Slave::isActivating() {
    return m_isActivating;
  }


  bool Slave::isAnyActiveSlaveResponded() {
    return m_isAnyActiveSlave;
  }


  Protocol::SlaveState Slave::getState() {
    return m_slaveState;
  }


  void Slave::approve(Protocol::CmdType cmdType) {
    if((Protocol::SlaveState::ACTIVE == m_slaveState) && (cmdType == m_cmdType)) {
      m_approveState = Protocol::ApproveState::APPROVED;
    }
  }

} // end namespace Eet