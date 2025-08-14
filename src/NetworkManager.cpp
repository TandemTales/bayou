#include "NetworkManager.h"
#include <iostream>

using namespace BayouBonanza;

NetworkManager::NetworkManager() : m_isConnected(false) {
    m_socket.setBlocking(false);
}

NetworkManager::~NetworkManager() {
    disconnect();
}

bool NetworkManager::connect(const std::string& serverIp, unsigned short port) {
    if (m_socket.connect(serverIp, port, sf::seconds(5)) == sf::Socket::Done) {
        m_isConnected = true;
        std::cout << "Successfully connected to server " << serverIp << ":" << port << std::endl;
        return true;
    }
    m_isConnected = false;
    std::cerr << "Failed to connect to server " << serverIp << ":" << port << std::endl;
    return false;
}

void NetworkManager::disconnect() {
    if (m_isConnected) {
        m_socket.disconnect();
        m_isConnected = false;
        std::cout << "Disconnected from server." << std::endl;
    }
}

void NetworkManager::send(sf::Packet& packet) {
    if (m_isConnected) {
        if (m_socket.send(packet) != sf::Socket::Done) {
            std::cerr << "Failed to send packet." << std::endl;
            // Handle error, maybe disconnect
        }
    }
}

void NetworkManager::receiveMessages() {
    if (!m_isConnected) return;

    sf::Packet packet;
    sf::Socket::Status status;
    while ((status = m_socket.receive(packet)) == sf::Socket::Done) {
        m_incomingPackets.push(packet);
    }

    if (status == sf::Socket::Disconnected) {
        std::cerr << "Server disconnected." << std::endl;
        disconnect();
    } else if (status == sf::Socket::Error) {
        std::cerr << "A network error occurred." << std::endl;
    }
}

bool NetworkManager::pollPacket(sf::Packet& packet) {
    if (m_incomingPackets.empty()) {
        return false;
    }
    packet = m_incomingPackets.front();
    m_incomingPackets.pop();
    return true;
}

bool NetworkManager::isConnected() const {
    return m_isConnected;
}
