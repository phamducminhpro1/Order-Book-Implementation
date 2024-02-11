/*
Author: Minh - Last modified: 29-5
This comment describes the implementation of a matching engine using C++

The matching engine manages orders through a Central Limit Order LimitBook (CLOB) with two sides: buy and sell.
The supported operations on orders are PULL, AMEND, and INSERT, which can be further explored in the main.hpp file or the problem description.
Upon execution, the order LimitBook generates sorted bid and ask price levels.

The main approach of the implementation is as follows:
A balanced binary tree is utilized to store price levels instead of individual orders, ensuring the prices are sorted in ascending order.
Each price level is represented by a linked list of orders.

To elaborate further:

An order ID to order mapping is maintained.
A map called "limitLookUp" is used to map a combination of symbol, side (BUY/SELL), and price limit to a "Limit" object, 
which represents a linked list of prices. Additional maps are employed to store prices sorted by sell and buy.

Review of the implementaion: 
This implementation is faster than having a balance tree with node as order that I implemented in the my pervious submission. 
It helps deletion, insert faster, if the price level exists before, it can be O(1)
One improvement, I can think of is to use arrays or circular buffers. This help to store data local and deletion lazy.
(This help to reduce memory fragementation, cache Locality reduce overhead because arrays and circualr buffers store contiguously)
*/

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <sstream>
#include <cassert> 
#include <map>
#include <bits/stdc++.h>

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
    int orderId;
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
    int agressiveOrderId;
    int passiveOrderId;

    MatchedOrders(string symbol, float price, int volume, int agressiveOrderId, int passiveOrderId)
    {
        this->symbol = symbol;
        this->price = price;
        this->volume = volume;
        this->agressiveOrderId = agressiveOrderId;
        this->passiveOrderId = passiveOrderId;
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
class Limit
{
public:
    float limitPrice;
    int totalVolume;
    string side;
    int size;
    list<StockOrder> listStock;

    Limit(float limitPrice, string side, int totalVolume)
    {
        this->limitPrice = limitPrice; 
        this->totalVolume = 0; //Initially, the total volum is 0 
        this->size = 0; //We maintain size. This is helpful when print out the left stock that previously unmatched
    }

    Limit() {}
};


class LimitBook {
public:
    map<float, Limit, greater<float>> buyTree;
    map<float, Limit> sellTree; 

    LimitBook() {}
};


/////////////////////////////////////////////////HELPER FUNCTION////////////////////////////////////////////////////////


// Convert a float to the string
// Input: float x
// Output: return string
string convertFloatToString(float x)
{
    stringstream s;
    s << x;
    string finalResult = s.str();
    return finalResult;
}

//  Convert a string to string ot a float
//  Input: float x
//  Output: string
float convertToFloat(const string &priceString)
{
    size_t dotPos = priceString.find('.');
    if (dotPos == string::npos)
    {
        // No decimal point found, assuming whole number
        return stof(priceString);
    }

    size_t digitsAfterDot = priceString.length() - dotPos - 1;
    if (digitsAfterDot > 4)
    {
        // More than 4 digits after the decimal point
        cout << "String input error, more than 4 number behind the decimal";
    }

    return stof(priceString);
}

// Input: String s
// Output: splits the input string s into multiple substring based on a delimiter as "," 
vector<string> splitString(string s)
{
    for (int i = 0; i < s.size(); ++i)
    {
        if (s[i] == ',')
            s[i] = ' ';
    }
    istringstream is(s);
    vector<string> finalResult;
    string x;
    while (is >> x)
        finalResult.push_back(x);
    return finalResult;
}

// The function matches an order (the curOrder is not added to the orderbook yet) from the input with the corresponding order in the opposite tree.
// It retrieves the best price from the opposite tree using limitSetSellLookUp or limitSetBuyLookUp.
// Once the price is obtained, it performs a limit lookup to find potential matches and evaluates whether they can be matched or not. 
// If a match is found, a trade is executed; otherwise, curOrder is not added to the limitbook if its volume is zero.
void matchOrder(StockOrder *curOrder, vector<string> &finalResult, unordered_map<int, StockOrder> &orderLookUp, unordered_map<string, LimitBook>& bookLookUp, set<string> &allSymbols)
{
    int orderId = curOrder->orderId;
    string symbol = curOrder->symbol;
    string side = curOrder->side;
    float price = curOrder->price;
    int volume = curOrder->volume;
    int timestamp = curOrder->timestamp;

    // Match the current order with the order from the opposite side
    string keyLimitLookUp = symbol + side + convertFloatToString(price);
    string oppositeSide = (side == "BUY") ? "SELL" : "BUY";
    //vecMatchedOrders is a vector to store all the matched orderd. This is in the format that helps to print out. 
    vector<MatchedOrders> vecMatchedOrders;

    while (true)
    {
        // If the opposite tree does not have any order then we break
        if ((side == "BUY" && bookLookUp[symbol].sellTree.size() == 0) || (side == "SELL" && bookLookUp[symbol].buyTree.size() == 0))
        {
            break;
        }
        Limit *potentialMatchLimit; 
        if (side == "BUY") {
            potentialMatchLimit = &bookLookUp[symbol].sellTree.begin()->second;
        } else {
            potentialMatchLimit = &bookLookUp[symbol].buyTree.begin()->second;
        } 
        
        // //Get the pontential match order
        StockOrder *potentialMatchOrder = &potentialMatchLimit->listStock.back(); 

        // If this is a match which the current side is sell and the price of sell is smaller or equal to the biggest buy price in the buy tree
        if (curOrder->side == "SELL" && potentialMatchOrder->price >= curOrder->price)
        {
            int tmp = min(potentialMatchOrder->volume, curOrder->volume);
            //update the volume of the curOrder
            curOrder->volume -= tmp;
            //Update the volume of the potentialMatchOrder (Matched in this case)
            potentialMatchOrder->volume -= tmp;
            //Update the limit. (This will helps to keep track of number of volume at that price)
            potentialMatchLimit->totalVolume -= tmp;
            //Put into the matches object, this will help with the printing
            MatchedOrders matches(symbol, potentialMatchOrder->price, tmp, curOrder->orderId, potentialMatchOrder->orderId);
            vecMatchedOrders.push_back(matches);
        }
        //If this is a match which the current side is buy and teh price of buy is bigger or equal to the smallest price in the sell tree
        else if (curOrder->side == "BUY" && potentialMatchOrder->price <= curOrder->price)
        {
            int tmp = min(potentialMatchOrder->volume, curOrder->volume);
            //update the volume of the curOrder
            curOrder->volume -= tmp;
            //Update the volume of the potentialMatchOrder (Matched in this case)
            potentialMatchOrder->volume -= tmp;
            //Update the limit. (This will helps to keep track of number of volume at that price)
            potentialMatchLimit->totalVolume -= tmp;
            //Put into the matches object, this will help with the printing
            MatchedOrders matches(symbol, potentialMatchOrder->price, tmp, curOrder->orderId, potentialMatchOrder->orderId);
            vecMatchedOrders.push_back(matches);
        }
        else
        {
            // There is no match anymore, so break; 
            break;
        }
        if (potentialMatchOrder->volume == 0)
        {
            // Hand in the remove all in here
            potentialMatchLimit->listStock.pop_back();
        }

        // There is a match, so now we need to remove it from the order from the order LimitBook, because the volume of it is 0
        if (curOrder->volume == 0)
        {
            break;
        }
    }

    //If the curOrer is not equal 0 then we add it to the order LimitBook. 
    if (curOrder->volume != 0)
    {
        Limit *potentialMatchLimit; 
        // Do I need to limit look up here. I think yes right, because I need to find the limit of the think or havign the order match the limit? 
        if (side == "BUY") {
            potentialMatchLimit = &bookLookUp[symbol].sellTree.begin()->second;
        } else {
            potentialMatchLimit = &bookLookUp[symbol].buyTree.begin()->second;
        } 
            cout << "efew" <<endl;
        potentialMatchLimit->listStock.push_front(*curOrder); 

    }

    // push the match to the result
    for (MatchedOrders &matchedOrders : vecMatchedOrders)
    {
        finalResult.push_back(matchedOrders.symbol + "," + convertFloatToString(matchedOrders.price) + "," + to_string(matchedOrders.volume) 
        + "," + to_string(matchedOrders.agressiveOrderId) + "," + to_string(matchedOrders.passiveOrderId));
    }
    return;
}



//////////////////////////////////////////////////////////QUERY FUNCTION ///////////////////////////////////////////////////////////////////////////


// Process Insert query 
// The function create curOrbject and call matchOrder to find if possible trade can happen
void processInsertQuery(vector<string> command, vector<string> &finalResult, unordered_map<int, StockOrder> &orderLookUp, unordered_map<string, LimitBook>& bookLookUp, set<string> &allSymbols)
{
    int orderId = stoi(command[1]);
    string symbol = command[2];
    string side = command[3];
    float price = convertToFloat(command[4]);
    int volume = stoi(command[5]);
    int timestamp = stoi(command[6]);

    StockOrder curOrder(orderId, symbol, side, price, volume, timestamp);
    //Check if we can match the order. (For the match order, in this case, 
    //in code will add the current order if after the match the volume is greater than 0)
    matchOrder(&curOrder, finalResult, orderLookUp, bookLookUp, allSymbols);
    return;
}

//Process amend query
//A pull removes the order from the order LimitBook. An amend changes the price and/or volume of the order. 
//An amend causes the order to lose time priority in the order LimitBook, unless the only change to the 
//orders that the volume is decreased. If the price of the order is amended, it needs to be re-evaluated for potential matches.
// void processAmendQuery(vector<string> command, vector<string> &finalResult, unordered_map<int, StockOrder> &orderLookUp, unordered_map<string, Limit> &limitLookUp, 
// unordered_map<string, set<float>> &limitSetSellLookUp, unordered_map<string, set<float, greater<float>>> &limitSetBuyLookUp, set<string> &allSymbols)
// {
//     int orderId = stoi(command[1]);
//     float priceChange = convertToFloat(command[2]);
//     int volumeChange = stoi(command[3]);
//     int timestamp = stoi(command[4]);
//     StockOrder curOrder;
//     assert(volumeChange != 0);
//     //If we can not find order
//     if (orderLookUp.find(orderId) != orderLookUp.end())
//     {
//         curOrder = orderLookUp[orderId];
//     }
//     else
//     {
//         cout << "Invalid amend request";
//     }

//     //If amend does not change the volume and price, then nothing will chante return
//     if (curOrder.volume == volumeChange && curOrder.price == priceChange)
//     {
//         return;
//     }

//     // If the volume decrease, and the price does not change. The priority of the order will remain the same, so 
//     if (curOrder.volume > volumeChange && curOrder.price == priceChange)
//     {
//         orderLookUp[orderId].volume = volumeChange;
//         limitLookUp[curOrder.symbol + curOrder.side + convertFloatToString(curOrder.price)].totalVolume -= (curOrder.volume - volumeChange);
//         return;
//     }

//     // The amend incrase the volume or changes the price, so remove the order from the order LimitBook
//     removeOrder(&curOrder, finalResult, orderLookUp, limitLookUp, limitSetSellLookUp, limitSetBuyLookUp, allSymbols);
//     //Update the order
//     curOrder.volume = volumeChange;
//     curOrder.price = priceChange;
//     //Match the order 
//     matchOrder(&curOrder, finalResult, orderLookUp, limitLookUp, limitSetSellLookUp, limitSetBuyLookUp, allSymbols);
//     return;
// }

// Pull query 
// The query will remove the order from the order LimitBook 
// The function will call the remove function
// void processPullQuery(vector<string> command, vector<string> &finalResult, unordered_map<int, StockOrder> &orderLookUp, unordered_map<string, Limit> 
// &limitLookUp, unordered_map<string, set<float>>& limitSetSellLookUp, unordered_map<string, set<float, greater<float>>>& limitSetBuyLookUp, set<string>& allSymbols)
// {
//     int orderId = stoi(command[1]);
//     if (orderLookUp.find(orderId) != orderLookUp.end())
//     {
//         StockOrder curOrder = orderLookUp[orderId];
//         removeOrder(&curOrder, finalResult, orderLookUp, limitLookUp, limitSetSellLookUp, limitSetBuyLookUp, allSymbols);
//     }
//     else
//     {
//         cout << "Invalid pull request";
//     }
// }

//The function get summary of bests matches of stocks sorted by symbols and print the remaining stock. 
void outPutPerSymbol(vector<string> &finalResult, unordered_map<string, LimitBook>& bookLookUp, set<string>& allSymbols)
{
    //Loop through all the symbols 
    for (const string &symbol : allSymbols)
    { 
        //If there exists stock of the symbol 
        if (!bookLookUp[symbol].buyTree.empty() || !bookLookUp[symbol].sellTree.empty())
        {
            finalResult.push_back("===" + symbol + "===");
        }  
        //Iterator of the both of the set. 
        auto it1 = bookLookUp[symbol].buyTree.begin();
        auto it2 = bookLookUp[symbol].sellTree.begin();
        while (it1 != bookLookUp[symbol].buyTree.end() || it2 != bookLookUp[symbol].sellTree.end())
        {
            //If there are orders on both side, then make the paris
            if (it1 != bookLookUp[symbol].buyTree.end() && it2 != bookLookUp[symbol].sellTree.end())
            {
                finalResult.push_back(convertFloatToString(it1->second.limitPrice) + "," + to_string(it1->second.totalVolume) + "," 
                + convertFloatToString(it2->second.limitPrice) + "," + to_string(it2->second.totalVolume));
                it1++;
                it2++;
            }
            //If the buy side has order stocks
            else if (it1 != bookLookUp[symbol].buyTree.end())
            {
                finalResult.push_back(convertFloatToString(it1->second.limitPrice) + "," + to_string(it1->second.totalVolume) + ",,");
                it1++;
            }
            //If the left side has order stocks 
            else
            {
                finalResult.push_back(",," + convertFloatToString(it2->second.limitPrice) + "," + to_string(it2->second.totalVolume));
                it2++;
            }
        }
    }
}

//Input vector<string> of commands 
//We loop through each command and find the matching functions with that commands. 
//At the end 
vector<string> run(vector<string> const &input)
{
    //Final result vector
    vector<string> finalResult;

    // This is the map from order id to stock order
    unordered_map<int, StockOrder> orderLookUp;

    // This is the map from symbol to LimitBook. 
    unordered_map<string, LimitBook> bookLookUp;

    //Set of all symbol 
    set<string> allSymbols;
    
    //Loop through the input
    for (int i = 0; i < input.size(); i++)
    {
        vector<string> command = splitString(input[i]);
        // Add the timestamp;
        command.push_back(to_string(i));
        if (command[0] == "INSERT")
        {
            processInsertQuery(command, finalResult, orderLookUp, bookLookUp, allSymbols);
        }
        else if (command[0] == "AMEND")
        {
            // processAmendQuery(command, finalResult, orderLookUp, bookLookUp, allSymbols);
        }
        else if (command[0] == "PULL")
        {
            // processPullQuery(command, finalResult, orderLookUp, bookLookUp, allSymbols);
        }
    } 
    //Print out the unmatched pairs before and individals group by symbol alphabetically 
    outPutPerSymbol(finalResult, bookLookUp, allSymbols);
    return finalResult;
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