/*
** EPITECH PROJECT, 2024
** Plazza
** File description:
** No file there , just an epitech header example .
** You can even have multiple lines if you want !
*/

#include "AbstractTicketBoard.hpp"
#include <utility>

const std::unordered_map<MessageType, std::pair<std::string, std::string>> MESSAGE_TYPE_TO_STRING = {
        {MessageType::NEW_TICKET, {NEW_TICKET_MESSAGE, NEW_TICKET_REGEX}},
        {MessageType::TICKET_REQUEST_ASSIGNMENT, {TICKET_REQUEST_ASSIGNMENT_MESSAGE, TICKET_REQUEST_ASSIGNMENT_REGEX}},
        {MessageType::TICKET_ASSIGNED, {TICKET_ASSIGNED_MESSAGE, TICKET_ASSIGNED_REGEX}},
        {MessageType::TICKET_MARKED_AS_DONE, {TICKET_MARKED_AS_DONE_MESSAGE, TICKET_MARKED_AS_DONE_REGEX}}
};

const std::unordered_map<MessageType, AbstractTicketBoard::TicketEventType>  AbstractTicketBoard::RELATION_MAP = {
    {MessageType::TICKET_ASSIGNED, AbstractTicketBoard::TicketEventType::ASSIGNED},
    {MessageType::TICKET_MARKED_AS_DONE, AbstractTicketBoard::TicketEventType::MARKED_AS_DONE},
    {MessageType::TICKET_REQUEST_ASSIGNMENT, AbstractTicketBoard::TicketEventType::REQUESTED_ASSIGNMENT},
    {MessageType::NEW_TICKET, AbstractTicketBoard::TicketEventType::ADDED}
};

AbstractTicketBoard::AbstractTicketBoard(Role role) {
    this->_role = role;
    this->_mutex = Mutex();
}

AbstractTicketBoard::~AbstractTicketBoard() {
    for (auto ticket : this->_tickets) {
        delete ticket;
    }
}

void AbstractTicketBoard::addTicket(Ticket *ticket) {
    _mutex.lock();
    this->_tickets.push_back(ticket);
    _mutex.unlock();
}

void AbstractTicketBoard::removeTicket(const UUID &ticketUUID) {
    _mutex.lock();
    this->_tickets.erase(std::remove_if(this->_tickets.begin(), this->_tickets.end(),
        [ticketUUID](const Ticket *ticket) {
            return ticket->getUuid() == ticketUUID;
        }), this->_tickets.end());
    _mutex.unlock();
}

static void supportsEventType(AbstractTicketBoard::Role role, AbstractTicketBoard::TicketEventType type) {
    const std::unordered_map<AbstractTicketBoard::Role, std::vector<AbstractTicketBoard::TicketEventType>> supportedEvents = {
        {AbstractTicketBoard::Role::MASTER, {AbstractTicketBoard::TicketEventType::MARKED_AS_DONE, AbstractTicketBoard::TicketEventType::REQUESTED_ASSIGNMENT}},
        {AbstractTicketBoard::Role::SLAVE,  {AbstractTicketBoard::TicketEventType::ASSIGNED, AbstractTicketBoard::TicketEventType::ADDED}}
    };

    bool found = std::find(supportedEvents.at(role).begin(), supportedEvents.at(role).end(), type) != supportedEvents.at(role).end();
    if (!found)
        throw AbstractTicketBoard::TicketBoardUnsupportedEventTypeException(type);
}

void AbstractTicketBoard::addListener(const TicketCallback &callback, TicketEventType type) {
    supportsEventType(this->_role, type);
    this->_callbacks[type].push_back(callback);
}

const std::vector<Ticket *> &AbstractTicketBoard::getTickets() const {
    return this->_tickets;
}

std::vector<Ticket *> &AbstractTicketBoard::getTickets() {
    return this->_tickets;
}

int AbstractTicketBoard::getSocket() const {
    return this->_socket;
}

void AbstractTicketBoard::addCommand(const Command &command) {
    for (std::size_t i = 0; i < command.getPizzas().size(); i++) {
        this->addTicket(new Ticket(command, i));
    }
}

void AbstractTicketBoard::removeAllTicketOfCommand(const Command &command) {
    _mutex.lock();
    this->_tickets.erase(std::remove_if(this->_tickets.begin(), this->_tickets.end(),
        [&command](const Ticket *ticket) {
            return ticket->getCommandUuid() == command.getUuid();
        }), this->_tickets.end());
    _mutex.unlock();
}

void AbstractTicketBoard::markTicketAsDone(const UUID &ticketUUID) {
    _mutex.lock();
    auto ticket = std::find_if(this->_tickets.begin(), this->_tickets.end(),
        [ticketUUID](const Ticket *ticket) {
            return ticket->getUuid() == ticketUUID;
        });
    if (ticket == this->_tickets.end()) {
        _mutex.unlock();
        throw AbstractTicketBoard::TicketBoardTicketNotFoundException(
                ticketUUID);
    }
    (*ticket)->setDone(true);
    (*ticket)->setBeingProcessed(false);
    _mutex.unlock();
}

void AbstractTicketBoard::markTicketAsBeingProcessed(const UUID &ticketUUID) {
    _mutex.lock();
    auto ticket = std::find_if(this->_tickets.begin(), this->_tickets.end(),
        [ticketUUID](const Ticket *ticket) {
            return ticket->getUuid() == ticketUUID;
        });
    if (ticket == this->_tickets.end()) {
        _mutex.unlock();
        throw AbstractTicketBoard::TicketBoardTicketNotFoundException(
                ticketUUID);
    }
    (*ticket)->setBeingProcessed(true);
    _mutex.unlock();
}

void AbstractTicketBoard::stop() {
    this->_isRunning = false;
}

Ticket *AbstractTicketBoard::getTicket(const UUID &ticketUUID) {
    _mutex.lock();
    auto ticket = std::find_if(this->_tickets.begin(), this->_tickets.end(),
        [ticketUUID](const Ticket *ticket) {
            return ticket->getUuid() == ticketUUID;
    });
    if (ticket == this->_tickets.end()) {
        _mutex.unlock();
        return nullptr;
    }
    _mutex.unlock();
    return *ticket.base();
}

std::vector<Ticket *> AbstractTicketBoard::getTickets(const UUID &commandUUID) {
    _mutex.lock();
    std::vector<Ticket *> tickets;
    for (auto ticket : this->_tickets) {
        if (ticket->getCommandUuid() == commandUUID) {
            tickets.push_back(ticket);
        }
    }
    _mutex.unlock();
    return tickets;
}

AbstractTicketBoard::TicketBoardException::TicketBoardException(std::string message) : std::exception(), _message(std::move(message)) {
}

const char *AbstractTicketBoard::TicketBoardException::what() const noexcept {
    return this->_message.c_str();
}

AbstractTicketBoard::TicketBoardUnsupportedEventTypeException::TicketBoardUnsupportedEventTypeException(TicketEventType type) : TicketBoardException("Unsupported event type: " + std::to_string(type)) {
}

AbstractTicketBoard::TicketBoardTicketNotFoundException::TicketBoardTicketNotFoundException(const UUID &ticketUUID) : TicketBoardException("Ticket not found: " + ticketUUID.toString()) {
}
