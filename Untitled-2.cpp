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