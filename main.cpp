#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <sstream>
#include <queue>
#include <unordered_map>

using namespace std;

namespace akuna::matching_engine {
    enum class OperationType : int {
        BUY,
        SELL,
        CANCEL,
        MODIFY,
        PRINT
    };

    enum class OrderType : int {
        IOC,
        GFD
    };

    enum class Side : int {
        BUY,
        SELL
    };

    class MatchigEngineUtility {
    public:
        static const OperationType &toOperationTypeEnum(string &operationTypeStringValue) {
            transform(operationTypeStringValue.begin(), operationTypeStringValue.end(),
                      operationTypeStringValue.begin(), ::toupper);
            switch (operationTypeStringValue.at(0)) {
                case 'B':
                    return checkValueAndReturnType(operationTypeStringValue, "BUY", OperationType::BUY);
                case 'S':
                    return checkValueAndReturnType(operationTypeStringValue, "SELL", OperationType::SELL);
                case 'C':
                    return checkValueAndReturnType(operationTypeStringValue, "CANCEL", OperationType::CANCEL);
                case 'M':
                    return checkValueAndReturnType(operationTypeStringValue, "MODIFY", OperationType::MODIFY);
                case 'P':
                    return checkValueAndReturnType(operationTypeStringValue, "PRINT", OperationType::PRINT);
                default:
                    throw invalid_argument(operationTypeStringValue);
            }
        }

        static const OrderType &toOrderTypeEnum(string &orderTypeStringValue) {
            transform(orderTypeStringValue.begin(), orderTypeStringValue.end(), orderTypeStringValue.begin(),
                      ::toupper);
            switch (orderTypeStringValue.at(0)) {
                case 'I':
                    return checkValueAndReturnType(orderTypeStringValue, "IOC", OrderType::IOC);
                case 'G':
                    return checkValueAndReturnType(orderTypeStringValue, "GFD", OrderType::GFD);
                default:
                    throw invalid_argument(orderTypeStringValue);
            }
        }

        static const Side &toSideEnum(string &sideStringValue) {
            transform(sideStringValue.begin(), sideStringValue.end(), sideStringValue.begin(),
                      ::toupper);
            switch (sideStringValue.at(0)) {
                case 'B':
                    return checkValueAndReturnType(sideStringValue, "BUY", Side::BUY);
                case 'S':
                    return checkValueAndReturnType(sideStringValue, "SELL", Side::SELL);
                default:
                    throw invalid_argument(sideStringValue);
            }
        }

        static const vector<string> processInput(string &line) {
            istringstream lineStringStream(trim(line));
            return vector<string>{istream_iterator<string>(lineStringStream), istream_iterator<string>()};
        }

    private:
        template<typename RV>
        static inline const RV &
        checkValueAndReturnType(const string &inputValue, const string &compareValue, const RV &returnType) {
            if (inputValue == compareValue) {
                return returnType;
            } else {
                throw invalid_argument(inputValue);
            }
        }

        static inline string &trimLeft(string &s) {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                            [](int ch) {
                                                return !std::isspace(ch);
                                            }));
            return s;
        }

        static inline string &trimRight(string &s) {
            s.erase(std::find_if(s.rbegin(), s.rend(),
                                 [](int ch) {
                                     return !std::isspace(ch);
                                 }).base(), s.end());
            return s;
        }

        static inline string &trim(string &s) {
            return trimLeft(trimRight(s));
        }
    };

    class OperationEvent {
        OperationType operationType;
    public:
        explicit OperationEvent(vector<string> &tokens) : operationType(
                MatchigEngineUtility::toOperationTypeEnum(tokens.at(0))) {}

        const OperationType &getOperationType() const {
            return operationType;
        }

    private:
        virtual void validateAttributes(const vector<string> &tokens) = 0;
    };

    class OrderEvent : public OperationEvent {
        OrderType orderType;
        unsigned int price;
        unsigned int quantity;
        string orderId;
    public:
        explicit OrderEvent(vector<string> &tokens) : OperationEvent(tokens),
                                                      orderType(MatchigEngineUtility::toOrderTypeEnum(tokens.at(1))),
                                                      price(static_cast<unsigned int>(stoul(tokens.at(2)))), quantity(
                        static_cast<unsigned int>(stoul(tokens.at(3)))), orderId(tokens.at(4)) {}

        const OrderType &getOrderType() {
            return orderType;
        }

        unsigned int getPrice() {
            return price;
        }

        unsigned int getQuantity() {
            return quantity;
        }

        void setQuantity(unsigned int newQuantity) {
            quantity = newQuantity;
        }

        const string &getOrderId() {
            return orderId;
        }
    };

    struct LessThanByPrice {
        bool operator()(const shared_ptr<OrderEvent> &lhs, const shared_ptr<OrderEvent> &rhs) const {
            return lhs->getPrice() < rhs->getPrice();
        }
    };

    class BuyEvent : public OrderEvent {
    public:
        explicit BuyEvent(vector<string> &tokens) : OrderEvent(tokens) {
            validateAttributes(tokens);
        }

    private:
        void validateAttributes(const vector<string> &tokens) override {
            if (getOperationType() != OperationType::BUY) {
                throw invalid_argument("Buy event cannot be created form " + tokens.at(0) + " operation type");
            }
        }
    };

    class SellEvent : public OrderEvent {
    public:
        explicit SellEvent(vector<string> &tokens) : OrderEvent(tokens) {
            validateAttributes(tokens);
        }

    private:
        void validateAttributes(const vector<string> &tokens) override {
            if (getOperationType() != OperationType::SELL) {
                throw invalid_argument("Sell event cannot be created form " + tokens.at(0) + " operation type");
            }
        }
    };

    class CancelEvent : public OperationEvent {
        string orderId;
    public:
        explicit CancelEvent(vector<string> &tokens) : OperationEvent(tokens), orderId(tokens.at(1)) {
            validateAttributes(tokens);
        }

        const string &getOrderId() {
            return orderId;
        }

    private:
        void validateAttributes(const vector<string> &tokens) override {
            if (getOperationType() != OperationType::CANCEL) {
                throw invalid_argument("Cancel event cannot be created form " + tokens.at(0) + " operation type");
            }
        }
    };

    class ModifyEvent : public OperationEvent {
        string orderId;
        Side side;
        unsigned int price;
        unsigned int quantity;
    public:
        explicit ModifyEvent(vector<string> &tokens) : OperationEvent(tokens), orderId(tokens.at(1)),
                                                       side(MatchigEngineUtility::toSideEnum(tokens.at(2))),
                                                       price(static_cast<unsigned int>(stoul(tokens.at(3)))), quantity(
                        static_cast<unsigned int>(stoul(tokens.at(4)))) {
            validateAttributes(tokens);
        }

        const Side &getSide() {
            return side;
        }

        unsigned int getPrice() {
            return price;
        }

        unsigned int getQuantity() {
            return quantity;
        }

        const string &getOrderId() {
            return orderId;
        }

    private:
        void validateAttributes(const vector<string> &tokens) override {
            if (getOperationType() != OperationType::MODIFY) {
                throw invalid_argument("Modify event cannot be created form " + tokens.at(0) + " operation type");
            }
        }
    };

    class PrintEvent : public OperationEvent {
    public:
        explicit PrintEvent(vector<string> &tokens) : OperationEvent(tokens) {
            validateAttributes(tokens);
        }

    private:
        void validateAttributes(const vector<string> &tokens) override {
            if (getOperationType() != OperationType::PRINT) {
                throw invalid_argument("Print event cannot be created form " + tokens.at(0) + " operation type");
            }
        }
    };

    typedef priority_queue<shared_ptr<OrderEvent>, vector<shared_ptr<OrderEvent>>, LessThanByPrice> OrderEventPriorityQueue;

    typedef unordered_map<unsigned int, deque<shared_ptr<OrderEvent>>> OrderEventPriceMap;

    typedef unordered_map<string, shared_ptr<OrderEvent>> OrderEventOrderMap;

    typedef unordered_map<Side, OrderEventPriorityQueue> SidePriceBookPriorityMap;

    typedef unordered_map<Side, OrderEventPriceMap> SidePriceBookMap;

    typedef unordered_map<string, unsigned long long int> OrderSequenceMap;

    class MatchingEngine {
        unsigned long long currentSequence = 0;
        OrderSequenceMap orderSequenceMap;
        OrderEventOrderMap orderEventOrderMap;
        SidePriceBookPriorityMap priceBookPriorityMap;
        SidePriceBookMap priceBookMap;
        OrderEventOrderMap replacedOrderEventOrderMap;

    public:
        void processEvent(const shared_ptr<OperationEvent> &operationEvent) {
            switch (operationEvent->getOperationType()) {
                case OperationType::BUY:
                    processEvent(dynamic_pointer_cast<OrderEvent>(operationEvent), Side::BUY);
                    break;
                case OperationType::SELL:
                    processEvent(dynamic_pointer_cast<OrderEvent>(operationEvent), Side::SELL);
                    break;
                case OperationType::PRINT:
                    processOperationEvent(operationEvent);
                    break;
                case OperationType::CANCEL:
                    processOperationEvent(operationEvent);
                    break;
                case OperationType::MODIFY:
                    processOperationEvent(operationEvent);
                    break;
            }
        }

        void processEvent(const shared_ptr<OrderEvent> &orderEvent) {
            if (orderEvent->getPrice() == 0 || orderEvent->getQuantity() == 0) return;

            switch (orderEvent->getOperationType()) {
                case OperationType::BUY:
                    processEvent(orderEvent, Side::BUY);
                    break;
                case OperationType::SELL:
                    processEvent(orderEvent, Side::SELL);
                    break;
                default:
                    throw invalid_argument("Operation type is not recognized");
            }
        }

        void printBook() {
            printSide(priceBookMap[Side::SELL], Side::SELL);
            printSide(priceBookMap[Side::BUY], Side::BUY);
        }

    private:
        void printQueue(const OrderEventPriceMap &map) {

            for (auto it : map) {
                unsigned int currentQuantity = 0;
                for (const auto &iti : it.second) {
                    currentQuantity += iti->getQuantity();
                }
                cout << it.first << " " << currentQuantity << endl;
            }
        }

        void printSide(const OrderEventPriceMap &sellMap, const Side &side) {
            switch (side) {
                case Side::SELL:
                    cout << "SELL:" << endl;
                    break;
                case Side::BUY:
                    cout << "BUY:" << endl;
                    break;
            }
            printQueue(sellMap);
        }

        void makeTrade(OrderEventPriorityQueue &queue, const shared_ptr<OrderEvent> &iocOrder) {
            do {
                auto order = queue.top();

                if (order->getQuantity() <= iocOrder->getQuantity()) {
                    cout << "TRADE " << order->getOrderId() << " " << order->getPrice() << " " << order->getQuantity()
                         << " "
                         << iocOrder->getOrderId() << " " << iocOrder->getPrice() << " " << order->getQuantity()
                         << endl;
                    iocOrder->setQuantity(iocOrder->getQuantity() - order->getQuantity());
                    order->setQuantity(0);
                } else {
                    cout << "TRADE " << order->getOrderId() << " " << order->getPrice() << " "
                         << iocOrder->getQuantity() << " "
                         << iocOrder->getOrderId() << " " << iocOrder->getPrice() << " " << iocOrder->getQuantity()
                         << endl;
                    order->setQuantity(order->getQuantity() - iocOrder->getQuantity());
                    iocOrder->setQuantity(0);
                }

                if (order->getQuantity() == 0) {
                    const Side side = order->getOperationType() == OperationType::BUY ? Side::BUY : Side::SELL;
                    cleanupOrder(order, side);
                    queue.pop();
                }
            } while (
                    (!queue.empty() && iocOrder->getOperationType() == OperationType::BUY ?
                     queue.top()->getPrice() <= iocOrder->getPrice() :
                     queue.top()->getPrice() >= iocOrder->getPrice()) &&
                    iocOrder->getQuantity() > 0 && queue.top()->getQuantity() > 0);
        }

        bool isReplacedOrder(const shared_ptr<OrderEvent> &replacedOrder, const shared_ptr<OrderEvent> &priorityOrder) {
            return replacedOrder != nullptr && priorityOrder != nullptr &&
                   replacedOrder->getOrderId() == priorityOrder->getOrderId() &&
                   replacedOrder->getOperationType() == priorityOrder->getOperationType() &&
                   replacedOrder->getPrice() == priorityOrder->getPrice() &&
                   replacedOrder->getQuantity() == priorityOrder->getQuantity();
        }

        void makeTrade(OrderEventPriorityQueue &leftQueue, OrderEventPriorityQueue &rightQueue) {
            do {
                auto leftOrder = leftQueue.top();
                auto rightOrder = rightQueue.top();
                auto leftOrderSeq = orderSequenceMap[leftOrder->getOrderId()];
                auto rightOrderSeq = orderSequenceMap[rightOrder->getOrderId()];

                if (orderEventOrderMap[leftOrder->getOrderId()] == nullptr ||
                    isReplacedOrder(replacedOrderEventOrderMap[leftOrder->getOrderId()], leftOrder)) {
                    leftQueue.pop();
                    replacedOrderEventOrderMap.erase(leftOrder->getOrderId());
                    continue;
                }

                shared_ptr<OrderEvent> testOrder = replacedOrderEventOrderMap[rightOrder->getOrderId()];
                if (orderEventOrderMap[rightOrder->getOrderId()] == nullptr ||
                    isReplacedOrder(replacedOrderEventOrderMap[rightOrder->getOrderId()], rightOrder)) {
                    rightQueue.pop();
                    replacedOrderEventOrderMap.erase(rightOrder->getOrderId());
                    continue;
                }

                if (leftOrder->getQuantity() <= rightOrder->getQuantity()) {
                    cout << "TRADE "
                         << (leftOrderSeq > rightOrderSeq ? rightOrder->getOrderId() : leftOrder->getOrderId()) << " "
                         << (leftOrderSeq > rightOrderSeq ? rightOrder->getPrice() : leftOrder->getPrice()) << " "
                         << leftOrder->getQuantity() << " "
                         << (leftOrderSeq > rightOrderSeq ? leftOrder->getOrderId() : rightOrder->getOrderId()) << " "
                         << (leftOrderSeq > rightOrderSeq ? leftOrder->getPrice() : rightOrder->getPrice()) << " "
                         << leftOrder->getQuantity() << " " << endl;
                    rightOrder->setQuantity(rightOrder->getQuantity() - leftOrder->getQuantity());
                    leftOrder->setQuantity(0);
                } else {
                    cout << "TRADE "
                         << (leftOrderSeq > rightOrderSeq ? rightOrder->getOrderId() : leftOrder->getOrderId()) << " "
                         << (leftOrderSeq > rightOrderSeq ? rightOrder->getPrice() : leftOrder->getPrice()) << " "
                         << rightOrder->getQuantity() << " "
                         << (leftOrderSeq > rightOrderSeq ? leftOrder->getOrderId() : rightOrder->getOrderId()) << " "
                         << (leftOrderSeq > rightOrderSeq ? leftOrder->getPrice() : rightOrder->getPrice()) << " "
                         << rightOrder->getQuantity() << " " << endl;
                    leftOrder->setQuantity(leftOrder->getQuantity() - rightOrder->getQuantity());
                    rightOrder->setQuantity(0);
                }

                if (leftOrder->getQuantity() == 0) {
                    const Side side = leftOrder->getOperationType() == OperationType::BUY ? Side::BUY : Side::SELL;
                    cleanupOrder(leftOrder, side);
                    leftQueue.pop();
                }

                if (rightOrder->getQuantity() == 0) {
                    const Side side = rightOrder->getOperationType() == OperationType::BUY ? Side::BUY : Side::SELL;
                    cleanupOrder(rightOrder, side);
                    rightQueue.pop();
                }
            } while (!rightQueue.empty() && !leftQueue.empty() &&
                     (rightQueue.top()->getOperationType() == OperationType::BUY ?
                      leftQueue.top()->getPrice() <= rightQueue.top()->getPrice() :
                      leftQueue.top()->getPrice() >= rightQueue.top()->getPrice()));
        }

        void cleanupOrder(const shared_ptr<OrderEvent> &order, const Side &side) {
            auto priceQueue = priceBookMap[side][order->getPrice()];
            auto iter = std::find_if(priceQueue.begin(), priceQueue.end(),
                                     [&](const shared_ptr<OrderEvent> &event) -> bool {
                                         return event->getOrderId() == order->getOrderId();
                                     });

            if (iter != priceQueue.end()) {
                priceQueue.erase(iter);
            }

            if (priceQueue.empty()) {
                priceBookMap[side].erase(order->getPrice());
            }
            orderSequenceMap.erase(order->getOrderId());
            orderEventOrderMap.erase(order->getOrderId());
        }

        void processIocEvent(OrderEventPriorityQueue &sellQueue,
                             OrderEventPriorityQueue &buyQueue,
                             const shared_ptr<OrderEvent> &iocOrder) {
            shared_ptr<OrderEvent> buyOrder;
            if (!buyQueue.empty()) {
                buyOrder = buyQueue.top();
            }
            shared_ptr<OrderEvent> sellOrder;
            if (!sellQueue.empty()) {
                sellOrder = sellQueue.top();
            }

            if (sellOrder != nullptr && buyOrder != nullptr && iocOrder != nullptr &&
                (sellOrder->getPrice() <= iocOrder->getPrice() ||
                 sellOrder->getPrice() <= buyOrder->getPrice())) {
                if (buyOrder->getPrice() >= iocOrder->getPrice()) {
                    processGfdEvent(sellQueue, buyQueue);
                    makeTrade(sellQueue, iocOrder);
                } else {
                    makeTrade(sellQueue, iocOrder);
                    processGfdEvent(sellQueue, buyQueue);
                }
            } else if (sellOrder != nullptr && iocOrder != nullptr && sellOrder->getPrice() <= iocOrder->getPrice()) {
                makeTrade(sellQueue, iocOrder);
            }
        }

        void processGfdEvent(OrderEventPriorityQueue &sellQueue, OrderEventPriorityQueue &buyQueue) {
            if (!sellQueue.empty() && !buyQueue.empty() &&
                sellQueue.top()->getPrice() <= buyQueue.top()->getPrice()) {
                makeTrade(sellQueue, buyQueue);
            }
        }

        void processEvent(const shared_ptr<OrderEvent> &event, const Side &side) {
            switch (event->getOrderType()) {
                case OrderType::GFD:
                    priceBookPriorityMap[side].push(event);
                    priceBookMap[side][event->getPrice()].push_back(event);
                    orderEventOrderMap[event->getOrderId()] = event;
                    orderSequenceMap[event->getOrderId()] = currentSequence++;
                    processGfdEvent(priceBookPriorityMap[Side::SELL], priceBookPriorityMap[Side::BUY]);
                    break;
                case OrderType::IOC:
                    processIocEvent(priceBookPriorityMap[Side::SELL], priceBookPriorityMap[Side::BUY], event);
                    break;
            }
        }

        void processOperationEvent(const shared_ptr<OperationEvent> &event) {
            switch (event->getOperationType()) {
                case OperationType::PRINT: {
                    printBook();
                    break;
                }
                case OperationType::CANCEL : {
                    shared_ptr<CancelEvent> cancelEvent = dynamic_pointer_cast<CancelEvent>(event);
                    shared_ptr<OrderEvent> originalCancelEvent = orderEventOrderMap[cancelEvent->getOrderId()];
                    if (originalCancelEvent != nullptr) {
                        const Side side =
                                originalCancelEvent->getOperationType() == OperationType::BUY ? Side::BUY : Side::SELL;
                        cleanupOrder(originalCancelEvent, side);
                    }
                    break;
                }
                case OperationType::MODIFY : {
                    shared_ptr<ModifyEvent> modifyEvent = dynamic_pointer_cast<ModifyEvent>(event);
                    shared_ptr<OrderEvent> originalModifyEvent = orderEventOrderMap[modifyEvent->getOrderId()];
                    if (originalModifyEvent != nullptr) {
                        shared_ptr<OrderEvent> copyEvent = duplicateOrderEvent(originalModifyEvent);
                        replacedOrderEventOrderMap.insert(
                                make_pair(copyEvent->getOrderId(), copyEvent));
                        shared_ptr<OrderEvent> replacedOrder = replacedOrderEventOrderMap[modifyEvent->getOrderId()];
                        const Side side =
                                originalModifyEvent->getOperationType() == OperationType::BUY ? Side::BUY : Side::SELL;
                        cleanupOrder(originalModifyEvent, side);

                        shared_ptr<OrderEvent> newEvent = createNewOrderEvent(modifyEvent, replacedOrder);
                        processEvent(newEvent);
                    }
                    break;
                }
                default:
                    throw invalid_argument("Operation type is not recognized");
            }
        }

        const shared_ptr<OrderEvent>
        createNewOrderEvent(shared_ptr<ModifyEvent> &modifyEvent, shared_ptr<OrderEvent> &originalModifyEvent) const {
            vector<string> orderRecord;
            orderRecord.emplace_back(modifyEvent->getSide() == Side::BUY ? "BUY" : "SELL");
            orderRecord.push_back(originalModifyEvent->getOrderType() == OrderType::GFD ? "GFD" : "IOC");
            orderRecord.push_back(to_string(modifyEvent->getPrice()));
            orderRecord.push_back(to_string(modifyEvent->getQuantity()));
            orderRecord.push_back(originalModifyEvent->getOrderId());
            shared_ptr<OrderEvent> newEvent;
            if (modifyEvent->getSide() == Side::BUY) {
                newEvent = make_shared<BuyEvent>(orderRecord);
            } else {
                newEvent = make_shared<SellEvent>(orderRecord);
            }
            return newEvent;
        }

        const shared_ptr<OrderEvent>
        duplicateOrderEvent(shared_ptr<OrderEvent> &originalModifyEvent) const {
            vector<string> orderRecord;
            orderRecord.emplace_back(originalModifyEvent->getOperationType() == OperationType::BUY ? "BUY" : "SELL");
            orderRecord.emplace_back(originalModifyEvent->getOrderType() == OrderType::GFD ? "GFD" : "IOC");
            orderRecord.push_back(to_string(originalModifyEvent->getPrice()));
            orderRecord.push_back(to_string(originalModifyEvent->getQuantity()));
            orderRecord.emplace_back(originalModifyEvent->getOrderId());
            shared_ptr<OrderEvent> newEvent;
            if (originalModifyEvent->getOperationType() == OperationType::BUY) {
                newEvent = make_shared<BuyEvent>(orderRecord);
            } else {
                newEvent = make_shared<SellEvent>(orderRecord);
            }
            return newEvent;
        }
    };
}

using namespace akuna::matching_engine;

int main() {
    MatchingEngine matchingEngine;

    //vector<string> order1{"BUY", "GFD", "1000", "10", "order1"};
    //shared_ptr<OrderEvent> orderEvent1 = make_shared<BuyEvent>(order1);
    //matchingEngine.processEvent(orderEvent1);

    //vector<string> order2{"BUY", "GFD", "1000", "10", "order2"};
    //shared_ptr<OrderEvent> orderEvent2 = make_shared<BuyEvent>(order2);
    //matchingEngine.processEvent(orderEvent2);

    //vector<string> event1{"modify", "order1", "BUY", "1000", "20"};
    //shared_ptr<OperationEvent> operation1 = make_shared<ModifyEvent>(event1);
    //matchingEngine.processEvent(operation1);

    //vector<string> order3{"SELL", "GFD", "900", "20", "order3"};
    //shared_ptr<OrderEvent> orderEvent3 = make_shared<SellEvent>(order3);
    //matchingEngine.processEvent(orderEvent3);

    //vector<string> event1{"PRINT"};
    //shared_ptr<OperationEvent> operation1 = make_shared<PrintEvent>(event1);
    //matchingEngine.processEvent(operation1);

    //vector<string> event2{"Cancel", "order3"};
    //shared_ptr<OperationEvent> operation2 = make_shared<CancelEvent>(event2);
    //matchingEngine.processEvent(operation2);

    //vector<string> event3{"Cancel", "order3"};
    //shared_ptr<OperationEvent> operation3 = make_shared<CancelEvent>(event3);
    //matchingEngine.processEvent(operation3);

    std::string input;
    while (std::getline(std::cin, input)) { // to quit the program press ctrl-d
        vector<string> tokens = MatchigEngineUtility::processInput(input);
        try {
            switch (MatchigEngineUtility::toOperationTypeEnum(tokens.at(0))) {
                case OperationType::BUY: {
                    shared_ptr<OrderEvent> buyOrderEvent = make_shared<BuyEvent>(tokens);
                    matchingEngine.processEvent(buyOrderEvent);
                    break;
                }
                case OperationType::SELL: {
                    shared_ptr<OrderEvent> sellOrderEvent = make_shared<SellEvent>(tokens);
                    matchingEngine.processEvent(sellOrderEvent);
                    break;
                }
                case OperationType::CANCEL: {
                    shared_ptr<OperationEvent> cancelEvent = make_shared<CancelEvent>(tokens);
                    matchingEngine.processEvent(cancelEvent);
                    break;
                }
                case OperationType::PRINT: {
                    shared_ptr<OperationEvent> printEvent = make_shared<PrintEvent>(tokens);
                    matchingEngine.processEvent(printEvent);
                    break;
                }
                case OperationType::MODIFY: {
                    shared_ptr<OperationEvent> modifyEvent = make_shared<ModifyEvent>(tokens);
                    matchingEngine.processEvent(modifyEvent);
                    break;
                }
            }
        } catch (const invalid_argument &ia) {
            cout << ia.what() << endl;
        } catch (const out_of_range &oor) {
            cout << oor.what() << endl;
            cout << "Please check your input" << endl;
        } catch (...) {
            cout << "Unknown exception" << endl;
        }
    }
    return 0;
}