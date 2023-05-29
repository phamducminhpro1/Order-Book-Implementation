/*
The main approach of the implementation is as follows:
Having one balanced tree per side with orders sorted on price and insertion order. To delete nodes, keep a hash of order IDs to tree nodes.
This makes any insertion / deletion / find BBO take O(log n) time, where 'n' is the number of active orders.

To elaborate further:
- unordered_map<int, StockOrder> orderLookUp;
We have a map from order ID to StockOrder.

- map<string, set<StockOrder, customComparator>> symbolLookUp;
We have another map that maps a string to a set of orders, which are compared using a custom comparator.

- set<string> allSymbols;
// We have a set containing all symbols. This will be useful for the print function at the end.
*/

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <sstream>
#include <map>
using namespace std;

// Stock order Order.
//  It contains
//  - orderID,
//  - symbol: Symbol of the stocks
//  - side: string of either buy or sell
//  - price of the stock
//  - volume of the stock
//  - timestamp: the time when the order was created (Note the timestamp variable is the variable I created when I add to the stock)
//  - nextOrder: pointer to the next order in the linked list
//  - prevOrder: pointer to the previous order in the linked list
class StockOrder
{
public:
    int orderId = -1;
    string symbol;
    string side;
    float price;
    int volume;
    int timestamp;

    StockOrder(int orderId, string symbol, string side, float price, int volume, int timestamp)
    {
        this->orderId = orderId;
        this->symbol = symbol;
        this->side = side;
        this->price = price;
        this->volume = volume;
        this->timestamp = timestamp;
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
class MatchedOrders
{
public:
    string symbol;
    float price;
    int volume;
    int aggressive_order_id;
    int passive_order_id;

    MatchedOrders(string symbol, float price, int volume, int aggressive_order_id, int passive_order_id)
    {
        this->symbol = symbol;
        this->price = price;
        this->volume = volume;
        this->aggressive_order_id = aggressive_order_id;
        this->passive_order_id = passive_order_id;
    }
    MatchedOrders() {}
};

/*
This code defines a custom comparator struct named customComparator.
The struct overloads the function call operator to compare two StockOrder objects.
The comparison is based on the "side", "price", and "timestamp" attributes of the StockOrder objects.
The comparison logic differs based on whether the side is "BUY" or not.
If side is "BUY":
- Orders with higher prices are considered "less" and are placed earlier in the sorted container.
- If prices are equal, orders with older timestamps are considered "less".
If side is not "BUY":
- Orders with lower prices are considered "less" and are placed earlier in the sorted container.
- If prices are equal, orders with older timestamps are considered "less".
*/
struct customComparator
{
    bool operator()(StockOrder a, StockOrder b) const
    {
        if (a.side == "BUY")
        {
            if (a.price > b.price)
                return true;
            else if (a.price < b.price)
                return false;
            else if (a.timestamp < b.timestamp)
                return true;
            else
                return false;
        }
        else
        {
            if (a.price < b.price)
                return true;
            else if (a.price > b.price)
                return false;
            else if (a.timestamp < b.timestamp)
                return true;
            else
                return false;
        }
    }
};

// Convert a float to the string
// Input float x
// Output string
string convertFloatToString(float x)
{
    stringstream s;
    s << x;
    string result = s.str();
    return result;
}

//  Convert a string to string ot a float
//  Input float x
//  Output string
float convertToFloat(const std::string &priceString)
{
    size_t dotPos = priceString.find('.');
    if (dotPos == std::string::npos)
    {
        // No decimal point found, assuming whole number
        return std::stof(priceString);
    }

    size_t digitsAfterDot = priceString.length() - dotPos - 1;
    if (digitsAfterDot > 4)
    {
        // More than 4 digits after the decimal point
        cout << "String input error, more than 4 number behind the decimal";
    }

    return std::stof(priceString);
}

// The function help to s plit string s (each file in s sepearted by ,) to vector of string sepearted. 
vector<string> splitString(string s)
{
    for (int i = 0; i < s.size(); ++i)
    {
        if (s[i] == ',')
            s[i] = ' ';
    }
    istringstream is(s);
    vector<string> result;
    string x;
    while (is >> x)
        result.push_back(x);
    return result;
}

// The function match an order in the input with the orther order in the opposite tree
void findMatch(StockOrder *curOrder, vector<string> &result, unordered_map<int, StockOrder> &orderLookUp, map<string, set<StockOrder, customComparator>> &symbolLookUp, set<string> &allSymbols)
{
    //Get the data from curOrder
    string symbol = curOrder->symbol;
    string side = curOrder->side;
    int orderId = curOrder->orderId;
    string key = symbol + side;
    allSymbols.insert(symbol);

    //Insert the current order to the
    orderLookUp[orderId] = *curOrder;
    symbolLookUp[key].insert(*curOrder);

    //Find the key for the opposite side
    string oppositeSide;
    oppositeSide = (side == "BUY") ? "SELL" : "BUY";
    string oppositeKey = symbol + oppositeSide;

    //Vector to stored the match orders
    vector<MatchedOrders> vecMatchedOrders;
    while (true)
    {
        // If there is nothing in the opposite tree, break the loop
        if (symbolLookUp.find(oppositeKey) == symbolLookUp.end())
        {
            break;
        }

        // There is a potential match in the opposite tree, find the highest priority
        StockOrder potentialMatch = *symbolLookUp[oppositeKey].begin();

        //Delete current order from symbolLookUp
        //If the size of symbolLookUp[key] = 1, then we delete the key. 
        //There is no point in strong a key to an empty set 
        if (symbolLookUp[key].size() == 1)
        {
            symbolLookUp.erase(key);
        }
        else
        {
            symbolLookUp[key].erase(*curOrder);
        }
        orderLookUp.erase(orderId); 

        //Delete oppositeKey from symbolLookUp
        //If the size of symbolLookUp[key] = 1, then we delete the key. 
        if (symbolLookUp[oppositeKey].size() == 1)
        {
            symbolLookUp.erase(oppositeKey);
        }
        else
        {
            symbolLookUp[oppositeKey].erase(potentialMatch);
        }
        orderLookUp.erase(potentialMatch.orderId); 

        //If there is a match when the current order side is Sell and the price of sell is smaller than  or equal 
        //to the potential match. The potential match is the highest price in the buy tree
        if (curOrder->side == "SELL" && potentialMatch.price >= curOrder->price)
        {
            int tmp = min(potentialMatch.volume, curOrder->volume);
            curOrder->volume -= tmp;
            potentialMatch.volume -= tmp;
            MatchedOrders matches(symbol, potentialMatch.price, tmp, curOrder->orderId, potentialMatch.orderId);
            vecMatchedOrders.push_back(matches);
        }
        //If there is a match when the current order side is Buy and the price of buy is greater than  or equal 
        //to the potential match. The potential match is the lowest sell price in the sell tree
        else if (curOrder->side == "BUY" && potentialMatch.price <= curOrder->price)
        {
            int tmp = min(potentialMatch.volume, curOrder->volume);
            curOrder->volume -= tmp;
            potentialMatch.volume -= tmp;
            MatchedOrders matches(symbol, potentialMatch.price, tmp, curOrder->orderId, potentialMatch.orderId);
            vecMatchedOrders.push_back(matches);
        }
        //There is no match, so insert back to orderbook
        else
        {
            orderLookUp[orderId] = *curOrder;
            symbolLookUp[key].insert(*curOrder);
            orderLookUp[potentialMatch.orderId] = potentialMatch;
            symbolLookUp[oppositeKey].insert(potentialMatch);
            //There is no more match, so break
            break;
        }

        //If the potentialMatch is greater than 0, then add it to the orderbook
        if (potentialMatch.volume != 0)
        {
            orderLookUp[potentialMatch.orderId] = potentialMatch;
            symbolLookUp[oppositeKey].insert(potentialMatch);
        }

        //If the curOrder is greater than 0, then add it to the orderbook
        if (curOrder->volume != 0)
        {
            orderLookUp[orderId] = *curOrder;
            symbolLookUp[key].insert(*curOrder);
        }
        else
        {
            //The current order does not have any other match, so break
            break;
        }
    }
    //Put the result of the matched order in the result
    for (MatchedOrders &matchedOrders : vecMatchedOrders)
    {
        result.push_back(matchedOrders.symbol + "," + convertFloatToString(matchedOrders.price) + "," + to_string(matchedOrders.volume) + "," + to_string(matchedOrders.aggressive_order_id) + "," + to_string(matchedOrders.passive_order_id));
    }
    return;
}

// Process insert query
void processInsertQuery(vector<string> command, vector<string> &result, unordered_map<int, StockOrder> &orderLookUp, map<string, set<StockOrder, customComparator>> &symbolLookUp, set<string> &allSymbols)
{
    int orderId = stoi(command[1]);
    string symbol = command[2];
    string side = command[3];
    float price = convertToFloat(command[4]);
    int volume = stoi(command[5]);
    int timestamp = stoi(command[6]);
    StockOrder curOrder(orderId, symbol, side, price, volume, timestamp);
    findMatch(&curOrder, result, orderLookUp, symbolLookUp, allSymbols);
    return;
}

// Process amend query
void processAmendQuery(vector<string> command, vector<string> &result, unordered_map<int, StockOrder> &orderLookUp, map<string, set<StockOrder, customComparator>> &symbolLookUp, set<string> &allSymbols)
{
    int orderId = stoi(command[1]);
    float price = convertToFloat(command[2]);
    int volume = stoi(command[3]);
    int timestamp = stoi(command[4]);
    StockOrder curOrder;
    if (orderLookUp.find(orderId) != orderLookUp.end())
    {
        curOrder = orderLookUp[orderId];
    }
    else
    {
        cout << "Invalid amend request";
    }

    // Nothing changes
    if (curOrder.volume == volume && curOrder.price == price)
    {
        return;
    }
    // If it is only volume change then only needs to change the volume
    //  This means that the prices changes
    // If the price change then, revaluate and added, order in again again
    // Delete the order from the table
    string key = curOrder.symbol + curOrder.side;
    if (symbolLookUp[key].size() == 1)
    {
        symbolLookUp.erase(key);
    }
    else
    {
        symbolLookUp[key].erase(curOrder);
    }
    orderLookUp.erase(orderId);

    curOrder.price = price;
    if (curOrder.volume <= volume)
    {
        curOrder.timestamp = timestamp;
    }
    curOrder.volume = volume;

    orderLookUp[orderId] = curOrder;
    symbolLookUp[key].insert(curOrder);
    findMatch(&curOrder, result, orderLookUp, symbolLookUp, allSymbols);
}

// Process pull query
void processPullQuery(vector<string> command, unordered_map<int, StockOrder> &orderLookUp, map<string, set<StockOrder, customComparator>> &symbolLookUp, set<string> &allSymbols)
{
    int orderId = stoi(command[1]);
    if (orderLookUp.find(orderId) != orderLookUp.end())
    {
        StockOrder curOrder = orderLookUp[orderId];
        string symbol = curOrder.symbol + curOrder.side;
        symbolLookUp[symbol].erase(curOrder);
        orderLookUp.erase(orderId);
    }
    else
    {
        cout << "Invalid pull request";
    }
}

vector<string> run(vector<string> const &input)
{
    vector<string> result;
    unordered_map<int, StockOrder> orderLookUp;
    map<string, set<StockOrder, customComparator>> symbolLookUp;
    set<string> allSymbols;
    for (int i = 0; i < input.size(); i++)
    {
        vector<string> command = splitString(input[i]);
        // Add the timestamp;
        command.push_back(to_string(i));
        if (command[0] == "INSERT")
        {
            processInsertQuery(command, result, orderLookUp, symbolLookUp, allSymbols);
        }
        else if (command[0] == "AMEND")
        {
            processAmendQuery(command, result, orderLookUp, symbolLookUp, allSymbols);
        }
        else if (command[0] == "PULL")
        {
            processPullQuery(command, orderLookUp, symbolLookUp, allSymbols);
        }
    }

    //Print the best left match at the end sorted by symbol 
    for (const string &symbol : allSymbols)
    {
        string buyKey = symbol + "BUY";
        string sellKey = symbol + "SELL";
        map<float, int, std::greater<float>> buy;
        map<float, int> sell;
        if (symbolLookUp.find(buyKey) != symbolLookUp.end() || symbolLookUp.find(sellKey) != symbolLookUp.end())
        {
            result.push_back("===" + symbol + "===");
        }
        if (symbolLookUp.find(buyKey) != symbolLookUp.end())
        {
            for (auto it = symbolLookUp[buyKey].begin(); it != symbolLookUp[buyKey].end(); it++)
            {
                buy[it->price] += it->volume;
            }
        }
        if (symbolLookUp.find(sellKey) != symbolLookUp.end())
        {
            for (auto it = symbolLookUp[sellKey].begin(); it != symbolLookUp[sellKey].end(); it++)
            {
                sell[it->price] += it->volume;
            }
        }
        auto it1 = buy.begin();
        auto it2 = sell.begin();
        while (it1 != buy.end() || it2 != sell.end())
        {
            if (it1 != buy.end() && it2 != sell.end())
            {
                result.push_back(convertFloatToString(it1->first) + "," + to_string(it1->second) + "," + convertFloatToString(it2->first) + "," + to_string(it2->second));
                it1++;
                it2++;
            }
            else if (it1 != buy.end())
            {
                result.push_back(convertFloatToString(it1->first) + "," + to_string(it1->second) + ",,");
                it1++;
            }
            else
            {
                result.push_back(",," + convertFloatToString(it2->first) + "," + to_string(it2->second));
                it2++;
            }
        }
    }
    return result;
}

int main()
{
    int line = 0;
    cin >> line;
    vector<string> command;
    for (int i = 0; i < line; i++)
    {
        string tmp;
        cin >> tmp;
        command.push_back(tmp);
    }
    vector<string> tmp1 = run(command);
    for (string line : tmp1)
    {
        cout << line << endl;
    }
    return 0;
}