/*
Author: Minh - Last modified: 29-5
This comment describes the implementation of a matching engine using C++.

The matching engine manages orders through a Central Limit Order Book (CLOB) with two sides: buy and sell.
The supported operations on orders are PULL, AMEND, and INSERT, which can be further explored in the main.hpp file or the problem description.
Upon execution, the order book generates sorted bid and ask price levels.

The main approach of the implementation is as follows:
A balanced binary tree is utilized to store price levels instead of individual orders, ensuring the prices are sorted in ascending order.
Each price level is represented by a linked list of orders.

To elaborate further:

An order ID to order mapping is maintained.
A map called "limitLookUp" is used to map a combination of symbol, side (BUY/SELL), and price limit to a "Limit" object, which represents a linked list of prices.
Additional maps are employed to store prices sorted by sell and buy.
*/

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <sstream>
using namespace std;
using namespace std; 


//Stock order Order.
// It contains 
// - orderID,
// - symbol: Symbol of the stocks
// - side: string of either buy or sell
// - price of the stock
// - volume of the stock
// - timestamp: the time when the order was created (Note the timestamp variable is the variable I created when I add to the stock) 
// - nextOrder: pointer to the next order in the linked list
// - prevOrder: pointer to the previous order in the linked list
class StockOrder {
    public:
        int orderId; 
        string symbol;
        string side; 
        float price; 
        int volume;
        int timestamp;  
        StockOrder *nextOrder; 
        StockOrder *prevOrder;   
    
    StockOrder(int orderId, string symbol, string side, float price, int volume, int timestamp) {
        this->orderId = orderId; 
        this->symbol = symbol;
        this->side = side; 
        this->price = price; 
        this->volume = volume; 
        this->timestamp = timestamp;
        this->nextOrder = NULL; 
        this->prevOrder = NULL; 
    } 
    StockOrder() {}
};

// MatchedOrders is a class that represents the result of matching two orders.
// It contains the following member variables:
// - symbol: the symbol of the matched stock
// - price: the price at which the orders were matched
// - volume: the volume of the matched orders
// - aggressive_order_id: the identifier of the aggressive order (the order that initiated the match)
// - passive_order_id: the identifier of the passive order (the order that was matched against)
// The purpose of this class is to store the matched order details and facilitate printing out the result.
// It has a constructor that initializes the member variables based on the provided arguments, as well as a default constructor.
// Overall, this class is useful for keeping track of matched orders and presenting the result in a structured manner. 
class MatchedOrders {
    public: 
        string symbol; 
        float price; 
        int volume; 
        int aggfinalResultsive_order_id; 
        int passive_order_id;
      
    MatchedOrders(string symbol, float price, int volume, int aggfinalResultsive_order_id, int passive_order_id) {
        this->symbol = symbol;
        this->price = price; 
        this->volume = volume; 
        this->aggfinalResultsive_order_id = aggfinalResultsive_order_id; 
        this->passive_order_id = passive_order_id;
    }
    MatchedOrders() {}
};

// Limit is a class representing a price level in a trading system. It contains a linked list of orders.
// - limitPrice: the price level of the limit
// - totalVolume: the total volume (quantity) of orders at this price level
// - side: the side of the limit (BUY or SELL)
// - size: the number of orders in the linked list
// - headOrder: the head (first) order in the linked list
// - tailOrder: the tail (last) order in the linked list 
class Limit {
    public: 
        float limitPrice; 
        int totalVolume;
        string side;
        int size;  
        StockOrder* headOrder; 
        StockOrder* tailOrder;

    Limit(float limitPrice, string side, int totalVolume, StockOrder* headOrder, StockOrder* tailOrder) {
        this->limitPrice = limitPrice; 
        this->totalVolume = 0; 
        this->headOrder = headOrder; 
        this->tailOrder = tailOrder; 
        this->size = 0; 
    }

    Limit() {}
}; 


// Convert a float to the string
// Input float x 
// Output string 
string convertFloatToString(float x) {
    stringstream s;
    s<<x; 
    string finalResult=s.str(); 
    return finalResult; 
}

//Convert a string to string ot a float
// Input float x 
// Output string 
float convertToFloat(const std::string& priceString) {
    size_t dotPos = priceString.find('.');
    if (dotPos == std::string::npos) {
        // No decimal point found, assuming whole number
        return std::stof(priceString);
    }

    size_t digitsAfterDot = priceString.length() - dotPos - 1;
    if (digitsAfterDot > 4) {
        // More than 4 digits after the decimal point
        cout << "String input error, more than 4 number behind the decimal"; 
    }

    return std::stof(priceString);
}

//Split string s to vector of string 
vector<string> splitString(string s){
    for(int i = 0; i < s.size(); ++i){
        if(s[i] == ',') s[i] = ' ';
    }
    istringstream is(s);
    vector<string> finalResult;
    string x;
    while(is >> x) finalResult.push_back(x);
    return finalResult;   
}

void removeOrder(StockOrder* curOrder, vector<string>& finalResult, unordered_map<int, StockOrder>& orderLookUp, unordered_map<string, Limit>& limitLookUp, unordered_map<string, set<float>>& limitSetSellLookUp, unordered_map<string, set<float, greater<float>>>& limitSetBuyLookUp, set<string>& allSymbols) {
    int orderId = curOrder->orderId;
    orderLookUp.erase(orderId);
    string keyLimitLookUp = curOrder->symbol + curOrder->side + convertFloatToString(curOrder->price);
    Limit *curLimit = &limitLookUp[keyLimitLookUp];

    //Delete an element from a linked list. 
    curLimit->totalVolume -= curOrder->volume;
    curLimit->size -= 1; 
    if (curOrder->prevOrder == NULL) {
        curLimit->headOrder = curOrder->nextOrder;
        if (curOrder->nextOrder != NULL) {
            curOrder->nextOrder->prevOrder = NULL;
        } else {
            curLimit->tailOrder = NULL;
        }
    } else {
        curOrder->prevOrder->nextOrder = curOrder->nextOrder;
        if (curOrder->nextOrder != NULL) {
            curOrder->nextOrder->prevOrder = curOrder->prevOrder;
        } else {
            curLimit->tailOrder = curOrder->prevOrder;
        }
    }
    if (curLimit->size == 0) {
        limitLookUp.erase(keyLimitLookUp);
        if (curOrder->side == "BUY") {
            limitSetBuyLookUp[curOrder->symbol].erase(curLimit->limitPrice);
        } else {
            limitSetSellLookUp[curOrder->symbol].erase(curLimit->limitPrice);
        }
    }
}

void addOrder(StockOrder* curOrder, vector<string>& finalResult, unordered_map<int, StockOrder>& orderLookUp, unordered_map<string, Limit>& limitLookUp, unordered_map<string, set<float>>& limitSetSellLookUp, unordered_map<string, set<float, greater<float>>>& limitSetBuyLookUp, set<string>& allSymbols) {
    string symbol = curOrder->symbol;
    string side = curOrder->side; 
    int orderId = curOrder->orderId; 
    int volume = curOrder->volume; 
    float price = curOrder->price;

    orderLookUp[orderId] = *curOrder; 
    allSymbols.insert(symbol);
    string keyLimitLookUp = symbol + side + convertFloatToString(price); 

    if (limitLookUp.find(keyLimitLookUp) == limitLookUp.end()) {
        Limit curLimit(price, side, volume, &orderLookUp[orderId], &orderLookUp[orderId]); 
        limitLookUp[keyLimitLookUp] = curLimit; 
    } else {
        //Add the order to the beginning of the linked list 
        limitLookUp[keyLimitLookUp].headOrder->prevOrder = &orderLookUp[orderId];
        orderLookUp[orderId].nextOrder = limitLookUp[keyLimitLookUp].headOrder;
        orderLookUp[orderId] = orderLookUp[orderId];
        limitLookUp[keyLimitLookUp].headOrder = &orderLookUp[orderId];
    }
    limitLookUp[keyLimitLookUp].size += 1; 
    limitLookUp[keyLimitLookUp].totalVolume += volume;
    //Update the map 

    if (side == "BUY") {
        limitSetBuyLookUp[symbol].insert(price);
    } else {
        limitSetSellLookUp[symbol].insert(price);
    } 
} 

void matchOrder(StockOrder* curOrder, vector<string>& finalResult, unordered_map<int, StockOrder>& orderLookUp, unordered_map<string, Limit>& limitLookUp, unordered_map<string, set<float>>& limitSetSellLookUp, unordered_map<string, set<float, greater<float>>>& limitSetBuyLookUp, set<string>& allSymbols) {
    int orderId = curOrder->orderId;
    string symbol = curOrder->symbol; 
    string side = curOrder->side; 
    float price = curOrder->price;
    int volume = curOrder->volume; 
    int timestamp = curOrder->timestamp; 
    //Match the current order with the order from the opposite side 
    string keyLimitLookUp = symbol+side+convertFloatToString(price); 
    string oppositeSide = (side == "BUY") ? "SELL" : "BUY";
    vector<MatchedOrders> vecMatchedOrders;  
    while (true) {
        //No opposite price
        if ((side == "BUY" && limitSetSellLookUp[symbol].size() == 0) || (side == "SELL" && limitSetBuyLookUp[symbol].size() == 0)) {
            break; 
        } 
        float potentialMatchPrice;
        if (side == "BUY") {
            potentialMatchPrice = *limitSetSellLookUp[symbol].begin();
        } else {
            potentialMatchPrice = *limitSetBuyLookUp[symbol].begin();
        }
        string oppositekeyLimitLookUp = symbol + oppositeSide + convertFloatToString(potentialMatchPrice); 
        Limit* potentialMatchLimit = &limitLookUp[oppositekeyLimitLookUp]; 
        StockOrder* potentialMatchOrder = potentialMatchLimit->tailOrder; 
        if (curOrder->side == "SELL" && potentialMatchOrder->price >= curOrder->price) {
            int tmp = min(potentialMatchOrder->volume, curOrder->volume); 
            curOrder->volume -= tmp; 
            potentialMatchOrder->volume -= tmp; 
            potentialMatchLimit->totalVolume -= tmp; 
            MatchedOrders matches(symbol, potentialMatchOrder->price, tmp, curOrder->orderId, potentialMatchOrder->orderId); 
            vecMatchedOrders.push_back(matches);
        } else if (curOrder->side == "BUY" && potentialMatchOrder->price <= curOrder->price) {
            int tmp = min(potentialMatchOrder->volume, curOrder->volume); 
            curOrder->volume -= tmp; 
            potentialMatchOrder->volume -= tmp; 
            potentialMatchLimit->totalVolume -= tmp; 
            MatchedOrders matches(symbol, potentialMatchOrder->price, tmp, curOrder->orderId, potentialMatchOrder->orderId); 
            vecMatchedOrders.push_back(matches);
        } else { 
            //There is no match anymore, so break; 
            break;
        }  
        if (potentialMatchOrder->volume == 0) {
            //Hand in the remove all in here
            removeOrder(potentialMatchOrder, finalResult, orderLookUp, limitLookUp, limitSetSellLookUp, limitSetBuyLookUp, allSymbols);
        } 
        
        //There is a match, so now we need to remove it from the order from the order book. 
        if (curOrder->volume == 0) {
            break;
        }
        
    }
    if (curOrder->volume != 0) {
        addOrder(curOrder,finalResult, orderLookUp, limitLookUp, limitSetSellLookUp, limitSetBuyLookUp, allSymbols); 
    }
    for (MatchedOrders& matchedOrders : vecMatchedOrders) {
        finalResult.push_back(matchedOrders.symbol + "," + convertFloatToString(matchedOrders.price) + "," + to_string(matchedOrders.volume) + "," +  to_string(matchedOrders.aggfinalResultsive_order_id) + "," + to_string(matchedOrders.passive_order_id));
    }
    return; 
}

void processInsertQuery(vector<string> command, vector<string>& finalResult, unordered_map<int, StockOrder>& orderLookUp, unordered_map<string, Limit>& limitLookUp, unordered_map<string, set<float>>& limitSetSellLookUp, unordered_map<string, set<float, greater<float>>>& limitSetBuyLookUp, set<string>& allSymbols) {
    int orderId = stoi(command[1]); 
    string symbol = command[2];
    string side = command[3];
    float price = convertToFloat(command[4]); 
    int volume = stoi(command[5]);
    int timestamp = stoi(command[6]); 
    //Current order
    StockOrder curOrder(orderId, symbol, side, price, volume, timestamp);
    
    //I have not added the order yet. 
    //It will added in the matchOrder
    //Add order
    matchOrder(&curOrder, finalResult, orderLookUp, limitLookUp, limitSetSellLookUp, limitSetBuyLookUp, allSymbols);
    return;    
}
  
void processAmendQuery(vector<string> command, vector<string>& finalResult, unordered_map<int, StockOrder>& orderLookUp, unordered_map<string, Limit>& limitLookUp, unordered_map<string, set<float>>& limitSetSellLookUp, unordered_map<string, set<float, greater<float>>>& limitSetBuyLookUp, set<string>& allSymbols) {
    int orderId = stoi(command[1]); 
    float price_change = convertToFloat(command[2]); 
    int volume_change = stoi(command[3]);
    int timestamp = stoi(command[4]);
    StockOrder curOrder; 

    if (orderLookUp.find(orderId) != orderLookUp.end()) {
        curOrder = orderLookUp[orderId]; 
    } else {
        cout << "Invalid amend request";
    }  

    if (curOrder.volume == volume_change && curOrder.price == price_change) {
        return; 
    }

    if (curOrder.volume > volume_change && curOrder.price == price_change) {
        orderLookUp[orderId].volume = volume_change;
        limitLookUp[curOrder.symbol + curOrder.side + convertFloatToString(curOrder.price)].totalVolume -= (curOrder.volume - volume_change);
        return;  
    }  

    removeOrder(&curOrder, finalResult, orderLookUp, limitLookUp, limitSetSellLookUp, limitSetBuyLookUp, allSymbols); 
    curOrder.volume = volume_change;
    curOrder.price = price_change; 
    matchOrder(&curOrder, finalResult, orderLookUp, limitLookUp, limitSetSellLookUp, limitSetBuyLookUp, allSymbols);
    return; 
}  

void processPullQuery(vector<string> command, vector<string>& finalResult, unordered_map<int, StockOrder> orderLookUp, unordered_map<string, Limit>& limitLookUp, unordered_map<string, set<float>>& limitSetSellLookUp, unordered_map<string, set<float, greater<float>>> limitSetBuyLookUp, set<string> allSymbols) {
    int orderId = stoi(command[1]);
    if (orderLookUp.find(orderId) != orderLookUp.end()) {
        StockOrder curOrder = orderLookUp[orderId]; 
        removeOrder(&curOrder, finalResult, orderLookUp, limitLookUp, limitSetSellLookUp, limitSetBuyLookUp, allSymbols);
    } else {
        cout << "Invalid pull request";
    }
}


void outPutPerSymbol(vector<string>& finalResult, unordered_map<string, Limit>& limitLookUp, unordered_map<string, set<float>>& limitSetSellLookUp, unordered_map<string, set<float, greater<float>>> limitSetBuyLookUp, set<string> allSymbols) {
    for (const string& symbol : allSymbols) {
        if (limitSetBuyLookUp[symbol].size() != 0 || limitSetSellLookUp[symbol].size() != 0) {
            finalResult.push_back("===" + symbol + "===");
        } 
        auto it1 = limitSetBuyLookUp[symbol].begin(); 
        auto it2 = limitSetSellLookUp[symbol].begin(); 
        while (it1 != limitSetBuyLookUp[symbol].end() || it2 != limitSetSellLookUp[symbol].end()) {
            if (it1 != limitSetBuyLookUp[symbol].end() && it2 != limitSetSellLookUp[symbol].end()) {
               finalResult.push_back(convertFloatToString(*it1) + "," + to_string(limitLookUp[symbol + "BUY" + convertFloatToString(*it1)].totalVolume) + "," + convertFloatToString(*it2) + "," + to_string(limitLookUp[symbol + "SELL" + convertFloatToString(*it2)].totalVolume));
               it1++; 
               it2++; 
            } else if (it1 != limitSetBuyLookUp[symbol].end()) {
                finalResult.push_back(convertFloatToString(*it1) + "," + to_string(limitLookUp[symbol + "BUY" + convertFloatToString(*it1)].totalVolume) + ",,");
                it1++;
            } else {
                finalResult.push_back( ",," + convertFloatToString(*it2) + "," + to_string(limitLookUp[symbol + "SELL" + convertFloatToString(*it2)].totalVolume));
                it2++;  
            }
        }
    }
}

vector<string> run(vector<string> const& input) {
    vector<string> finalResult;
    unordered_map<int, StockOrder> orderLookUp; 
    //This string will be SymBol + BUY/SELL + price Limit
    unordered_map<string, Limit> limitLookUp; 
    //This string will be SymBol
    unordered_map<string, set<float>> limitSetSellLookUp;
    unordered_map<string, set<float, greater<float>>> limitSetBuyLookUp;
    set<string> allSymbols;
    allSymbols.insert("abc");
    for (int i = 0; i < input.size(); i++) {
        vector<string> command = splitString(input[i]);
        //Add the timestamp; 
        command.push_back(to_string(i));
        if (command[0] == "INSERT") {
            processInsertQuery(command, finalResult, orderLookUp, limitLookUp, limitSetSellLookUp, limitSetBuyLookUp, allSymbols);
        } else if (command[0] == "AMEND") {
            processAmendQuery(command, finalResult, orderLookUp, limitLookUp, limitSetSellLookUp, limitSetBuyLookUp, allSymbols);  
        } else if (command[0] == "PULL") {
            processPullQuery(command, finalResult, orderLookUp, limitLookUp, limitSetSellLookUp, limitSetBuyLookUp, allSymbols);
        }
    } 
    outPutPerSymbol(finalResult, limitLookUp, limitSetSellLookUp, limitSetBuyLookUp, allSymbols);
    return finalResult;
}


int main() {
    int line = 0;
    cin >> line;  
    vector<string> command; 
    for (int i = 0; i < line; i++) {
        string tmp; 
        cin >> tmp; 
        command.push_back(tmp); 
    }
    vector<string> tmp1 = run(command);
    for (string line : tmp1) {
        cout << line << endl;
    } 
    // cout << limitLookUp["AAPLBUY14.235"].headOrder->nextOrder->orderId; 
    // cout << "Fuck " << endl;
    return 0; 
}