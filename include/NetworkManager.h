#pragma once

#include <SFML/Network.hpp>
#include <string>
#include <queue>
#include "NetworkProtocol.h"

namespace BayouBonanza {

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    bool connect(const std::string& serverIp, unsigned short port);
    void disconnect();

    void send(sf::Packet& packet);
    void receiveMessages();

    bool pollPacket(sf::Packet& packet);

    bool isConnected() const;

private:
    sf::TcpSocket m_socket;
    bool m_isConnected;
    std::queue<sf::Packet> m_incomingPackets;
};

} // namespace BayouBonanza
